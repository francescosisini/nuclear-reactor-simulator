# Nuclear Reactor Simulator

A modular nuclear reactor simulator written in C.

The purpose of this project is to make the complexity of a nuclear power system visible and understandable, both from a physical and from a systemic point of view.

It is intended as an educational and experimental tool through which interested users can become familiar with the main physical phenomena, understand the relationships between the different components, and progressively clarify how a nuclear reactor works as an integrated system.

The project initially focuses on the reactor itself, rather than on the complete electricity-generation cycle.

## Project Philosophy


Its main goal is to construct an explicit model of:

* what exists inside the simulated system;
* how physical entities are represented;
* how components are connected;
* what quantities flow between them;
* which physical laws transform their state.

The software architecture should therefore reflect the ontology of the physical system.

## Ontological Model

The project is based on four main layers.

### 1. Physical Entities

C structures represent the fundamental entities of the simulation.

Examples include:

* enriched uranium;
* depleted uranium;
* fission products;
* neutron populations;
* water and steam;
* control rods;
* reactor components;
* thermal and mechanical state variables.

These structures describe the entities and their physical state.

```c
typedef struct {
    double mass;
    double enrichment;
    double temperature;
} NuclearFuel;
```

The exact structures will evolve as the physical model becomes more detailed.

### 2. Pipes and Physical Flows

The connections between components are represented through a pipe-like abstraction inspired by C and POSIX pipes.

A pipe represents a physical connection through which a quantity can move between two components.

Possible flows include:

* coolant mass flow;
* steam flow;
* thermal energy transfer;
* material transport;
* neutron-related quantities;
* control signals.

These pipes are not necessarily operating-system pipes. They are domain-specific C structures representing the topology and state of the physical network.

```text
Reactor core
    |
    | coolant pipe
    v
Heat exchanger
    |
    | steam pipe
    v
Secondary circuit
```

The purpose of this abstraction is to make the physical connections explicit without forcing the whole C program into an artificial block-diagram programming model.

### 3. Physical Processes

Normal C functions implement the physical laws and transformations acting on entities and flows.

Examples include:

* nuclear fission;
* neutron absorption;
* radioactive decay;
* heat generation;
* heat exchange;
* water heating;
* phase transition;
* coolant circulation;
* control-rod insertion;
* reactor shutdown.

```c
void exchange_heat(
    ThermalBody *source,
    ThermalBody *destination,
    double time_step
);
```

The C language remains responsible for program structure, memory management, control flow, and numerical computation.

The physical architecture is represented through the data model rather than by forcing every software module to expose a generic input and output function.

## Conceptual Structure

The main conceptual structure of the project is:

```text
entities
    ↓
physical state
    ↓
connections and pipes
    ↓
physical processes
    ↓
system evolution
```

In more implementation-oriented terms:

```text
C structs      → physical entities
pipes          → connections and flows
C functions    → physical laws and transformations
JSON/GeoJSON   → configuration, persistence and spatial representation
```

## Persistence and Configuration

The simulator will use text-based formats for configuration and persistence.

### JSON

JSON files will be used to describe:

* physical entities;
* reactor parameters;
* initial conditions;
* simulation scenarios;
* component configuration;
* saved simulation states;
* simulation output metadata.

Example:

```json
{
  "fuel": {
    "material": "uranium_dioxide",
    "enrichment": 0.035,
    "temperature_k": 900.0
  }
}
```

### GeoJSON

GeoJSON will be used when the spatial arrangement of the system is relevant.

It may describe:

* component positions;
* pipe paths;
* reactor zones;
* coolant channels;
* plant layout;
* spatially distributed measurements;
* relationships between simulation objects and geographical or geometrical data.

The first versions may use simplified coordinates rather than the geometry of a real nuclear facility.

## Initial Physical Scope

The first development stages will model the reactor as an open dynamic system.

Inside the reactor core, nuclear fission produces thermal energy. The intensity of the chain reaction depends on the neutron population and on the effective multiplication factor (k_{\mathrm{eff}}).

Under normal steady operating conditions:

```text
k_eff = 1
```

This means that the neutron population and reactor power remain approximately constant.

Heat is removed by a coolant entering the core at a lower temperature and leaving it at a higher temperature.

The operating objective is to maintain:

* controlled neutron multiplication;
* stable thermal power;
* adequate coolant flow;
* fuel and coolant temperatures within design ranges;
* a balance between generated and removed heat.

## Development Approach

The simulator will grow through progressively more detailed physical models.

Planned stages include:

1. neutron multiplication;
2. point reactor kinetics;
3. delayed neutrons;
4. thermal power generation;
5. decay heat;
6. fuel and coolant temperatures;
7. thermal feedback on reactivity;
8. control rods;
9. SCRAM and emergency shutdown;
10. coolant and steam flows;
11. coupled reactor components;
12. spatial and plant-layout representation.

Each model should clearly document:

* equations;
* assumptions;
* physical units;
* numerical methods;
* validity limits;
* reference sources;
* verification tests.

## Repository Structure

```text
nuclear-reactor-simulator/
├── docs/
├── examples/
├── include/
├── src/
│   └── main.c
├── tests/
├── .gitignore
├── LICENSE
├── Makefile
└── README.md
```

The structure will evolve as the ontology and simulation engine are developed.

## Build

Compile the project with:

```bash
make
```

Run it with:

```bash
./nuclear_sim
```

Clean generated files with:

```bash
make clean
```

## Current Status

The project is in its initial design phase.

The current work focuses on:

* defining the physical ontology;
* designing entity structures;
* defining pipe and flow abstractions;
* separating physical state from physical processes;
* defining JSON and GeoJSON persistence formats;
* preparing the first minimal reactor model.

## Intended Use

This project is designed for:

* education;
* conceptual exploration;
* software architecture experiments;
* simplified reactor-physics simulations;
* study of coupled physical systems.

It is not intended for reactor operation, engineering certification, safety analysis, plant licensing, or prediction of the behaviour of a specific nuclear facility.

## License

See the `LICENSE` file.
