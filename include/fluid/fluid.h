#ifndef FLOW_H
#define FLOW_H

#include <stddef.h>

typedef struct FlowElement FlowElement;

/* Stato esposto all'uscita di ogni elemento. */
typedef struct {
    double mass_flow;      /* kg/s */
    double pressure;       /* Pa */
    double temperature;    /* K */
    double density;        /* kg/m^3 */
} FlowState;

typedef enum {
    FLOW_PASSIVE_PIPE,
    FLOW_PUMP,
    FLOW_PRESSURE_GAUGE
} FlowElementType;

/* Funzioni locali di un elemento. */
typedef void (*FlowEvaluateFn)(FlowElement *self, double dt);
typedef void (*FlowDestroyFn)(FlowElement *self);

struct FlowElement {
    const char *name;
    FlowElementType type;

    /* L'input non è posseduto: è l'output dell'elemento a monte. */
    FlowElement *upstream;

    /* Doppio buffer per aggiornamenti simultanei. */
    FlowState output;
    FlowState next_output;

    /* Perdita di massa comune a tutti gli elementi. */
    double leak_fraction;      /* 0.0 = nessuna perdita; 1.0 = perdita totale */
    double leaked_mass_flow;   /* kg/s, risultato dell'ultimo evaluate */

    void *data;

    FlowEvaluateFn evaluate;
    FlowDestroyFn destroy;
};

/* Collegamenti e ciclo di simulazione. */
int flow_connect(FlowElement *source, FlowElement *destination);
const FlowState *flow_input(const FlowElement *element);
void flow_evaluate(FlowElement *element, double dt);
void flow_commit(FlowElement *element);
void flow_set_initial_output(FlowElement *element, FlowState state);
void flow_set_leak(FlowElement *element, double leak_fraction);
void flow_destroy(FlowElement *element);

/* Costruttori degli elementi minimi. */
FlowElement *passive_pipe_create(
    const char *name,
    double pressure_loss
);

FlowElement *pump_create(
    const char *name,
    double pressure_gain,
    int enabled
);

FlowElement *pressure_gauge_create(
    const char *name
);

/* Comandi e letture specifiche. */
void pump_set_enabled(FlowElement *pump, int enabled);
void pump_set_pressure_gain(FlowElement *pump, double pressure_gain);
double pressure_gauge_read(const FlowElement *gauge);

#endif
