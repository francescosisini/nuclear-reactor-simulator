#ifndef FLOW_H
#define FLOW_H

#include <stddef.h>

typedef struct FlowNode FlowNode;
typedef struct FlowElement FlowElement;
typedef struct FlowCircuit FlowCircuit;

/*
 * Proprietà del fluido trasportato.
 * Per ora il fluido è incomprimibile.
 */
typedef struct {
    double temperature;    /* K */
    double density;        /* kg/m^3 */
    double viscosity;      /* Pa s */
} FluidProperties;

/*
 * La pressione appartiene ai nodi.
 * Un elemento collega due nodi e impone una relazione
 * tra la loro differenza di pressione e la portata.
 */
struct FlowNode {
    const char *name;
    double pressure;       /* Pa */
};

typedef enum {
    FLOW_PASSIVE_PIPE,
    FLOW_PUMP,
    FLOW_PRESSURE_GAUGE
} FlowElementType;

/*
 * Convenzione:
 *
 * delta_p = P_out - P_in
 *
 * Pompa: delta_p > 0
 * Tubo : delta_p < 0
 * Gauge: delta_p = 0
 */
typedef double (*FlowDeltaPFn)(
    FlowElement *self,
    double mass_flow,
    const FluidProperties *fluid
);

typedef void (*FlowAdvanceFn)(
    FlowElement *self,
    double dt
);

typedef void (*FlowDestroyFn)(
    FlowElement *self
);

struct FlowElement {
    const char *name;
    FlowElementType type;

    FlowNode *inlet;
    FlowNode *outlet;

    void *data;

    FlowDeltaPFn delta_p;
    FlowAdvanceFn advance;
    FlowDestroyFn destroy;
};

/*
 * Primo solver: un singolo anello chiuso in serie.
 *
 * La pressione assoluta non viene fatta "girare" attraverso
 * gli elementi. Un nodo viene preso come riferimento usando
 * la pressione iniziale del circuito; da quel nodo il profilo
 * viene ricostruito tramite i delta-P locali.
 *
 * La dinamica della portata dipende dal residuo di pressione:
 *
 * I_h d(m_dot)/dt = sum(delta_p_i)
 */
struct FlowCircuit {
    FlowElement **elements;
    size_t element_count;

    FlowNode *reference_node;
    double reference_pressure;       /* Pa */

    FluidProperties fluid;

    double mass_flow;                /* kg/s */
    double hydraulic_inertance;      /* parametro lumped provvisorio */
    double pressure_residual;        /* Pa */
};

/* Nodi */
FlowNode *flow_node_create(
    const char *name,
    double initial_pressure
);

void flow_node_destroy(
    FlowNode *node
);

/* Elementi */
FlowElement *passive_pipe_create(
    const char *name,
    FlowNode *inlet,
    FlowNode *outlet,
    double length,
    double diameter,
    double roughness
);

FlowElement *pump_create(
    const char *name,
    FlowNode *inlet,
    FlowNode *outlet,
    double target_pressure_gain,
    double pressure_time_constant,
    int enabled
);

FlowElement *pressure_gauge_create(
    const char *name,
    FlowNode *inlet,
    FlowNode *outlet
);

void flow_destroy(
    FlowElement *element
);

/* Pompa */
void pump_set_enabled(
    FlowElement *pump,
    int enabled
);

void pump_set_pressure_gain(
    FlowElement *pump,
    double pressure_gain
);

void pump_set_pressure_time_constant(
    FlowElement *pump,
    double pressure_time_constant
);

double pump_get_current_pressure_gain(
    const FlowElement *pump
);

/* Gauge */
double pressure_gauge_read(
    const FlowElement *gauge
);

/* Circuito chiuso in serie */
int flow_circuit_init(
    FlowCircuit *circuit,
    FlowElement **elements,
    size_t element_count,
    FlowNode *reference_node,
    FluidProperties fluid,
    double initial_mass_flow,
    double hydraulic_inertance
);

void flow_circuit_step(
    FlowCircuit *circuit,
    double dt
);

#endif
