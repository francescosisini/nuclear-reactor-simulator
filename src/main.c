#include <stdio.h>
#include <stdlib.h>

#include "fluid/fluid.h"

int main(void)
{
    const double dt = 0.1;

    FlowElement *pump = pump_create(
        "primary_pump",
        200000.0,   /* aumento di pressione: Pa */
        1           /* pompa accesa */
    );

    FlowElement *pipe = passive_pipe_create(
        "primary_pipe",
        150000.0    /* perdita di pressione: Pa */
    );

    FlowElement *gauge = pressure_gauge_create(
        "pressure_gauge"
    );

    if (pump == NULL || pipe == NULL || gauge == NULL) {
        fprintf(stderr, "Errore nella creazione del circuito.\n");

        flow_destroy(pump);
        flow_destroy(pipe);
        flow_destroy(gauge);

        return EXIT_FAILURE;
    }

    /* Circuito chiuso. */
    int status1 = flow_connect(pump, pipe);
    int status2 = flow_connect(pipe, gauge);
    int status3 = flow_connect(gauge, pump);

    if (status1) {
        fprintf(stderr, "Errore: pump -> pipe\n");
    }

    if (status2) {
        fprintf(stderr, "Errore: pipe -> gauge\n");
    }

    if (status3) {
        fprintf(stderr, "Errore: gauge -> pump\n");
    }

    if (status1 || status2 || status3) {
        flow_destroy(pump);
        flow_destroy(pipe);
        flow_destroy(gauge);

        return EXIT_FAILURE;
    }

    /* Stato iniziale uniforme del fluido. */
    FlowState initial_state = {
        .mass_flow = 10.0,       /* kg/s */
        .pressure = 1000000.0,   /* Pa */
        .temperature = 293.15,   /* K */
        .density = 998.0         /* kg/m^3 */
    };

    flow_set_initial_output(pump, initial_state);
    flow_set_initial_output(pipe, initial_state);
    flow_set_initial_output(gauge, initial_state);

    FlowElement *circuit[] = {
        pump,
        pipe,
        gauge
    };

    const size_t element_count =
        sizeof(circuit) / sizeof(circuit[0]);

    printf("Hydraulic circuit simulation\n\n");

    for (int step = 0; step < 12; ++step) {
        /*
         * Prima fase: ogni elemento legge soltanto gli output
         * correnti e calcola il proprio next_output.
         */
        for (size_t i = 0; i < element_count; ++i) {
            flow_evaluate(circuit[i], dt);
        }

        /*
         * Seconda fase: tutti i nuovi output diventano visibili
         * contemporaneamente.
         */
        for (size_t i = 0; i < element_count; ++i) {
            flow_commit(circuit[i]);
        }

        printf(
            "step %2d | pump: %9.0f Pa | pipe: %9.0f Pa"
            " | gauge: %9.0f Pa\n",
            step + 1,
            pump->output.pressure,
            pipe->output.pressure,
            pressure_gauge_read(gauge)
        );
    }

    flow_destroy(gauge);
    flow_destroy(pipe);
    flow_destroy(pump);

    return EXIT_SUCCESS;
}
