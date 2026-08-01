#include <stdio.h>
#include <stdlib.h>

#include "flow.h"

int main(void)
{
    const double dt = 0.1;

    const double initial_pressure = 1000000.0; /* Pa */

    const FluidProperties water = {
        .temperature = 293.15,
        .density = 998.0,
        .viscosity = 0.001002
    };

    const double pipe_length = 10.0;
    const double pipe_diameter = 0.10;
    const double pipe_roughness = 0.000045;

    const double pump_delta_p = 1000.0;
    const double pump_tau = 5.0;

    /*
     * Ancora provvisorio: verrà derivato fisicamente in seguito.
     */
    const double hydraulic_inertance = 100000.0;

    const int max_steps = 50000;

    FlowNode *n0 =
        flow_node_create(
            "pump_in",
            initial_pressure
        );

    FlowNode *n1 =
        flow_node_create(
            "pump_out",
            initial_pressure
        );

    FlowNode *n2 =
        flow_node_create(
            "pipe1_out",
            initial_pressure
        );

    FlowNode *n3 =
        flow_node_create(
            "gauge_out",
            initial_pressure
        );

    if (n0 == NULL ||
        n1 == NULL ||
        n2 == NULL ||
        n3 == NULL) {

        fprintf(stderr,
            "Errore nella creazione dei nodi.\n");

        return EXIT_FAILURE;
    }

    FlowElement *pump =
        pump_create(
            "primary_pump",
            n0,
            n1,
            pump_delta_p,
            pump_tau,
            1
        );

    FlowElement *pipe1 =
        passive_pipe_create(
            "pipe_1",
            n1,
            n2,
            pipe_length,
            pipe_diameter,
            pipe_roughness
        );

    FlowElement *gauge =
        pressure_gauge_create(
            "pressure_gauge",
            n2,
            n3
        );

    FlowElement *pipe2 =
        passive_pipe_create(
            "pipe_2",
            n3,
            n0,
            pipe_length,
            pipe_diameter,
            pipe_roughness
        );

    if (pump == NULL ||
        pipe1 == NULL ||
        gauge == NULL ||
        pipe2 == NULL) {

        fprintf(stderr,
            "Errore nella creazione del circuito.\n");

        return EXIT_FAILURE;
    }

    FlowElement *elements[] = {
        pump,
        pipe1,
        gauge,
        pipe2
    };

    FlowCircuit circuit;

    int status =
        flow_circuit_init(
            &circuit,
            elements,
            sizeof(elements) / sizeof(elements[0]),
            n0,
            water,
            0.0,
            hydraulic_inertance
        );

    if (status) {
        fprintf(stderr,
            "Errore nell'inizializzazione del circuito.\n");
        return EXIT_FAILURE;
    }

    printf("Hydraulic circuit simulation\n\n");

    printf(
        "Initial pressure = %.0f Pa\n"
        "Pump target dP = %.0f Pa\n"
        "Initial mass flow = %.6f kg/s\n\n",
        initial_pressure,
        pump_delta_p,
        circuit.mass_flow
    );

    for (int step = 1;
         step <= max_steps;
         ++step) {

        flow_circuit_step(
            &circuit,
            dt
        );

        if (step <= 20 ||
            step % 1000 == 0) {

            printf(
                "step %5d | "
                "m_dot = %9.6f kg/s | "
                "dPpump = %8.2f Pa | "
                "residual = %9.3f Pa | "
                "Pin = %11.2f | "
                "Pout = %11.2f | "
                "Ppipe1 = %11.2f | "
                "Pgauge = %11.2f\n",
                step,
                circuit.mass_flow,
                pump_get_current_pressure_gain(pump),
                circuit.pressure_residual,
                n0->pressure,
                n1->pressure,
                n2->pressure,
                pressure_gauge_read(gauge)
            );
        }
    }

    printf(
        "\nFinal mass flow: %.6f kg/s\n"
        "Final pressure residual: %.6f Pa\n",
        circuit.mass_flow,
        circuit.pressure_residual
    );

    flow_destroy(pump);
    flow_destroy(pipe1);
    flow_destroy(gauge);
    flow_destroy(pipe2);

    flow_node_destroy(n0);
    flow_node_destroy(n1);
    flow_node_destroy(n2);
    flow_node_destroy(n3);

    return EXIT_SUCCESS;
}
