#include "flow.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
    double length;
    double diameter;
    double roughness;
} PassivePipeData;

typedef struct {
    double target_pressure_gain;
    double current_pressure_gain;
    double pressure_time_constant;
    int enabled;
} PumpData;

typedef struct {
    double measured_pressure;
} PressureGaugeData;

static double abs_double(double value)
{
    return value < 0.0 ? -value : value;
}

static FlowElement *flow_element_create(
    const char *name,
    FlowElementType type,
    FlowNode *inlet,
    FlowNode *outlet,
    size_t data_size,
    FlowDeltaPFn delta_p,
    FlowAdvanceFn advance
)
{
    FlowElement *element = calloc(1, sizeof(*element));

    if (element == NULL) {
        return NULL;
    }

    if (data_size > 0) {
        element->data = calloc(1, data_size);

        if (element->data == NULL) {
            free(element);
            return NULL;
        }
    }

    element->name = name;
    element->type = type;
    element->inlet = inlet;
    element->outlet = outlet;
    element->delta_p = delta_p;
    element->advance = advance;

    return element;
}

FlowNode *flow_node_create(
    const char *name,
    double initial_pressure
)
{
    FlowNode *node = calloc(1, sizeof(*node));

    if (node == NULL) {
        return NULL;
    }

    node->name = name;
    node->pressure = initial_pressure;

    return node;
}

void flow_node_destroy(
    FlowNode *node
)
{
    free(node);
}

static double pipe_cross_section(
    const PassivePipeData *pipe
)
{
    const double pi = 3.14159265358979323846;

    return
        pi
        * pipe->diameter
        * pipe->diameter
        / 4.0;
}

static double pipe_reynolds(
    const PassivePipeData *pipe,
    double mass_flow,
    const FluidProperties *fluid
)
{
    if (fluid == NULL ||
        fluid->density <= 0.0 ||
        fluid->viscosity <= 0.0 ||
        pipe->diameter <= 0.0) {
        return 0.0;
    }

    const double area =
        pipe_cross_section(pipe);

    if (area <= 0.0) {
        return 0.0;
    }

    const double velocity =
        mass_flow
        / (fluid->density * area);

    return
        fluid->density
        * abs_double(velocity)
        * pipe->diameter
        / fluid->viscosity;
}

static double pipe_friction_factor(
    const PassivePipeData *pipe,
    double reynolds
)
{
    if (reynolds <= 0.0) {
        return 0.0;
    }

    if (reynolds < 2300.0) {
        return 64.0 / reynolds;
    }

    const double relative_roughness =
        pipe->roughness / pipe->diameter;

    const double term =
        pow(relative_roughness / 3.7, 1.11)
        + 6.9 / reynolds;

    const double inv_sqrt_f =
        -1.8 * log10(term);

    if (inv_sqrt_f == 0.0) {
        return 0.0;
    }

    return
        1.0
        / (inv_sqrt_f * inv_sqrt_f);
}

static double passive_pipe_delta_p(
    FlowElement *self,
    double mass_flow,
    const FluidProperties *fluid
)
{
    PassivePipeData *pipe = self->data;

    if (pipe == NULL ||
        fluid == NULL ||
        pipe->length <= 0.0 ||
        pipe->diameter <= 0.0 ||
        fluid->density <= 0.0) {
        return 0.0;
    }

    const double area =
        pipe_cross_section(pipe);

    if (area <= 0.0) {
        return 0.0;
    }

    const double velocity =
        mass_flow
        / (fluid->density * area);

    const double reynolds =
        pipe_reynolds(
            pipe,
            mass_flow,
            fluid
        );

    const double friction_factor =
        pipe_friction_factor(
            pipe,
            reynolds
        );

    /*
     * Darcy-Weisbach.
     *
     * La perdita ha segno opposto alla direzione della portata.
     */
    const double loss =
        friction_factor
        * (pipe->length / pipe->diameter)
        * (
            fluid->density
            * velocity
            * abs_double(velocity)
            / 2.0
        );

    return -loss;
}

FlowElement *passive_pipe_create(
    const char *name,
    FlowNode *inlet,
    FlowNode *outlet,
    double length,
    double diameter,
    double roughness
)
{
    FlowElement *element =
        flow_element_create(
            name,
            FLOW_PASSIVE_PIPE,
            inlet,
            outlet,
            sizeof(PassivePipeData),
            passive_pipe_delta_p,
            NULL
        );

    if (element == NULL) {
        return NULL;
    }

    PassivePipeData *pipe =
        element->data;

    pipe->length = length;
    pipe->diameter = diameter;
    pipe->roughness = roughness;

    return element;
}

static void pump_advance(
    FlowElement *self,
    double dt
)
{
    PumpData *pump = self->data;

    const double target =
        pump->enabled
        ? pump->target_pressure_gain
        : 0.0;

    if (pump->pressure_time_constant > 0.0 &&
        dt < pump->pressure_time_constant) {

        pump->current_pressure_gain +=
            (target - pump->current_pressure_gain)
            * dt
            / pump->pressure_time_constant;
    }
    else {
        pump->current_pressure_gain =
            target;
    }
}

static double pump_delta_p(
    FlowElement *self,
    double mass_flow,
    const FluidProperties *fluid
)
{
    PumpData *pump = self->data;

    (void)mass_flow;
    (void)fluid;

    return pump->current_pressure_gain;
}

FlowElement *pump_create(
    const char *name,
    FlowNode *inlet,
    FlowNode *outlet,
    double target_pressure_gain,
    double pressure_time_constant,
    int enabled
)
{
    FlowElement *element =
        flow_element_create(
            name,
            FLOW_PUMP,
            inlet,
            outlet,
            sizeof(PumpData),
            pump_delta_p,
            pump_advance
        );

    if (element == NULL) {
        return NULL;
    }

    PumpData *pump =
        element->data;

    pump->target_pressure_gain =
        target_pressure_gain;

    pump->current_pressure_gain =
        0.0;

    pump->pressure_time_constant =
        pressure_time_constant;

    pump->enabled =
        enabled != 0;

    return element;
}

static double gauge_delta_p(
    FlowElement *self,
    double mass_flow,
    const FluidProperties *fluid
)
{
    (void)self;
    (void)mass_flow;
    (void)fluid;

    return 0.0;
}

FlowElement *pressure_gauge_create(
    const char *name,
    FlowNode *inlet,
    FlowNode *outlet
)
{
    return flow_element_create(
        name,
        FLOW_PRESSURE_GAUGE,
        inlet,
        outlet,
        sizeof(PressureGaugeData),
        gauge_delta_p,
        NULL
    );
}

void flow_destroy(
    FlowElement *element
)
{
    if (element == NULL) {
        return;
    }

    if (element->destroy != NULL) {
        element->destroy(element);
        return;
    }

    free(element->data);
    free(element);
}

void pump_set_enabled(
    FlowElement *element,
    int enabled
)
{
    if (element == NULL ||
        element->type != FLOW_PUMP) {
        return;
    }

    ((PumpData *)element->data)->enabled =
        enabled != 0;
}

void pump_set_pressure_gain(
    FlowElement *element,
    double pressure_gain
)
{
    if (element == NULL ||
        element->type != FLOW_PUMP) {
        return;
    }

    ((PumpData *)element->data)->target_pressure_gain =
        pressure_gain;
}

void pump_set_pressure_time_constant(
    FlowElement *element,
    double pressure_time_constant
)
{
    if (element == NULL ||
        element->type != FLOW_PUMP) {
        return;
    }

    ((PumpData *)element->data)->pressure_time_constant =
        pressure_time_constant;
}

double pump_get_current_pressure_gain(
    const FlowElement *element
)
{
    if (element == NULL ||
        element->type != FLOW_PUMP) {
        return 0.0;
    }

    return
        ((const PumpData *)element->data)
        ->current_pressure_gain;
}

double pressure_gauge_read(
    const FlowElement *element
)
{
    if (element == NULL ||
        element->type != FLOW_PRESSURE_GAUGE ||
        element->inlet == NULL) {
        return 0.0;
    }

    return element->inlet->pressure;
}

int flow_circuit_init(
    FlowCircuit *circuit,
    FlowElement **elements,
    size_t element_count,
    FlowNode *reference_node,
    FluidProperties fluid,
    double initial_mass_flow,
    double hydraulic_inertance
)
{
    if (circuit == NULL ||
        elements == NULL ||
        element_count == 0 ||
        reference_node == NULL ||
        hydraulic_inertance <= 0.0) {
        return -1;
    }

    circuit->elements = elements;
    circuit->element_count = element_count;
    circuit->reference_node = reference_node;
    circuit->reference_pressure =
        reference_node->pressure;
    circuit->fluid = fluid;
    circuit->mass_flow = initial_mass_flow;
    circuit->hydraulic_inertance =
        hydraulic_inertance;
    circuit->pressure_residual = 0.0;

    return 0;
}

static void flow_circuit_update_pressures(
    FlowCircuit *circuit
)
{
    /*
     * La pressione iniziale del circuito fissa il livello assoluto.
     * I componenti determinano solo i salti tra nodi.
     */
    circuit->reference_node->pressure =
        circuit->reference_pressure;

    double pressure =
        circuit->reference_pressure;

    for (size_t i = 0;
         i < circuit->element_count;
         ++i) {

        FlowElement *element =
            circuit->elements[i];

        if (element == NULL ||
            element->delta_p == NULL ||
            element->outlet == NULL) {
            continue;
        }

        const double delta_p =
            element->delta_p(
                element,
                circuit->mass_flow,
                &circuit->fluid
            );

        pressure += delta_p;

        /*
         * Se l'outlet coincide con il nodo di riferimento
         * non lo sovrascriviamo: la differenza residua è
         * precisamente la forza che accelera/decelera il fluido.
         */
        if (element->outlet !=
            circuit->reference_node) {
            element->outlet->pressure =
                pressure;
        }
    }
}

void flow_circuit_step(
    FlowCircuit *circuit,
    double dt
)
{
    if (circuit == NULL ||
        dt <= 0.0) {
        return;
    }

    /*
     * Prima evolvono gli elementi dinamici, per esempio
     * il delta-P reale della pompa durante startup/shutdown.
     */
    for (size_t i = 0;
         i < circuit->element_count;
         ++i) {

        FlowElement *element =
            circuit->elements[i];

        if (element != NULL &&
            element->advance != NULL) {
            element->advance(
                element,
                dt
            );
        }
    }

    /*
     * Residuo di pressione sull'intero giro.
     */
    double residual = 0.0;

    for (size_t i = 0;
         i < circuit->element_count;
         ++i) {

        FlowElement *element =
            circuit->elements[i];

        if (element != NULL &&
            element->delta_p != NULL) {

            residual +=
                element->delta_p(
                    element,
                    circuit->mass_flow,
                    &circuit->fluid
                );
        }
    }

    circuit->pressure_residual =
        residual;

    /*
     * Il residuo non viene accumulato nella pressione media:
     * accelera/decelera la portata.
     */
    circuit->mass_flow +=
        (residual
         / circuit->hydraulic_inertance)
        * dt;

    /*
     * Ricostruisce il profilo istantaneo di pressione
     * dai delta-P dei componenti.
     */
    flow_circuit_update_pressures(
        circuit
    );
}
