#include "fluid/fluid.h"

#include <stdlib.h>

typedef struct {
    double pressure_loss;
} PassivePipeData;

typedef struct {
    double pressure_gain;
    int enabled;
} PumpData;

typedef struct {
    double measured_pressure;
} PressureGaugeData;

static double clamp01(double value)
{
    if (value < 0.0) {
        return 0.0;
    }
    if (value > 1.0) {
        return 1.0;
    }
    return value;
}

static FlowElement *flow_element_create(
    const char *name,
    FlowElementType type,
    size_t data_size,
    FlowEvaluateFn evaluate
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
    element->evaluate = evaluate;
    element->leak_fraction = 0.0;

    return element;
}

static FlowState read_input_or_self(const FlowElement *self)
{
    const FlowState *input = flow_input(self);

    /*
     * Un elemento non ancora collegato conserva il proprio output.
     * È utile durante la costruzione e l'inizializzazione della rete.
     */
    return input != NULL ? *input : self->output;
}

static void apply_leak(FlowElement *self)
{
    const double fraction = clamp01(self->leak_fraction);
    const double incoming_mass_flow = self->next_output.mass_flow;

    self->leaked_mass_flow = incoming_mass_flow * fraction;
    self->next_output.mass_flow =
        incoming_mass_flow - self->leaked_mass_flow;
}

static void passive_pipe_evaluate(FlowElement *self, double dt)
{
    PassivePipeData *pipe = self->data;
    (void)dt;

    self->next_output = read_input_or_self(self);
    self->next_output.pressure -= pipe->pressure_loss;

    apply_leak(self);
}

static void pump_evaluate(FlowElement *self, double dt)
{
    PumpData *pump = self->data;
    (void)dt;

    self->next_output = read_input_or_self(self);

    if (pump->enabled) {
        self->next_output.pressure += pump->pressure_gain;
    }

    apply_leak(self);
}

static void pressure_gauge_evaluate(FlowElement *self, double dt)
{
    PressureGaugeData *gauge = self->data;
    (void)dt;

    self->next_output = read_input_or_self(self);
    gauge->measured_pressure = self->next_output.pressure;

    apply_leak(self);
}

int flow_connect(FlowElement *source, FlowElement *destination)
{
    if (source == NULL || destination == NULL) {
        return -1;
    }

    destination->upstream = source;
    return 0;
}

const FlowState *flow_input(const FlowElement *element)
{
    if (element == NULL || element->upstream == NULL) {
        return NULL;
    }

    return &element->upstream->output;
}

void flow_evaluate(FlowElement *element, double dt)
{
    if (element == NULL || element->evaluate == NULL) {
        return;
    }

    element->evaluate(element, dt);
}

void flow_commit(FlowElement *element)
{
    if (element == NULL) {
        return;
    }

    element->output = element->next_output;
}

void flow_set_initial_output(FlowElement *element, FlowState state)
{
    if (element == NULL) {
        return;
    }

    element->output = state;
    element->next_output = state;
}

void flow_set_leak(FlowElement *element, double leak_fraction)
{
    if (element == NULL) {
        return;
    }

    element->leak_fraction = clamp01(leak_fraction);
}

void flow_destroy(FlowElement *element)
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

FlowElement *passive_pipe_create(
    const char *name,
    double pressure_loss
)
{
    FlowElement *element = flow_element_create(
        name,
        FLOW_PASSIVE_PIPE,
        sizeof(PassivePipeData),
        passive_pipe_evaluate
    );

    if (element == NULL) {
        return NULL;
    }

    ((PassivePipeData *)element->data)->pressure_loss = pressure_loss;
    return element;
}

FlowElement *pump_create(
    const char *name,
    double pressure_gain,
    int enabled
)
{
    FlowElement *element = flow_element_create(
        name,
        FLOW_PUMP,
        sizeof(PumpData),
        pump_evaluate
    );

    if (element == NULL) {
        return NULL;
    }

    PumpData *pump = element->data;
    pump->pressure_gain = pressure_gain;
    pump->enabled = enabled != 0;

    return element;
}

FlowElement *pressure_gauge_create(const char *name)
{
    return flow_element_create(
        name,
        FLOW_PRESSURE_GAUGE,
        sizeof(PressureGaugeData),
        pressure_gauge_evaluate
    );
}

void pump_set_enabled(FlowElement *element, int enabled)
{
    if (element == NULL || element->type != FLOW_PUMP) {
        return;
    }

    ((PumpData *)element->data)->enabled = enabled != 0;
}

void pump_set_pressure_gain(
    FlowElement *element,
    double pressure_gain
)
{
    if (element == NULL || element->type != FLOW_PUMP) {
        return;
    }

    ((PumpData *)element->data)->pressure_gain = pressure_gain;
}

double pressure_gauge_read(const FlowElement *element)
{
    if (element == NULL || element->type != FLOW_PRESSURE_GAUGE) {
        return 0.0;
    }

    return ((const PressureGaugeData *)element->data)->measured_pressure;
}
