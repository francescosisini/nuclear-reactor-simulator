# `flow` Library: Physical Model and Hydraulic Circuit Architecture

## 1. Purpose

This library aims to represent thermo-hydraulic circuits in a way that
is **physically readable**, even before becoming numerically
sophisticated.

The guiding principle is to clearly separate three physical concepts:

-   **pressure** → a property of **nodes**;
-   **mass flow rate** → a property of the **flow in a branch/circuit**;
-   **components** → impose a local relationship between pressure
    difference and flow rate.

The fundamental conceptual scheme is:

``` text
          m_dot →
   ┌───────────────────┐
   │                   │
   ▼                   │
 [N0] -- PUMP -- [N1] -- PIPE1 -- [N2] -- GAUGE -- [N3] -- PIPE2 --┘
  P0              P1               P2                P3
```

Pressure is therefore not "transported" from one element to the next as
a state that repeatedly circulates around the loop.

Each node has a pressure potential; each component determines the
pressure difference between its terminals.

------------------------------------------------------------------------

## 2. Electrical Analogy

A useful analogy is an electrical circuit:

  Hydraulics                               Electricity
  ---------------------------------------- ---------------------------------------
  pressure (P)                             electric potential (V)
  pressure difference (`\Delta `{=tex}P)   voltage difference (`\Delta `{=tex}V)
  mass flow rate (`\dot `{=tex}m)          current (I)
  pump imposing (`\Delta `{=tex}P)         voltage source
  pressure loss                            voltage drop across a resistance

An ideal pump with an assigned pressure rise **does not add pressure at
every circulation around the loop**.

It simply imposes the constraint:

```math
P\_{out}-P\_{in}=`\Delta `{=tex}P\_{pump}
```

exactly as an ideal voltage source imposes:

\[ V\_+-V\_-=E \]

This is the key idea that eliminated the artificial drift of absolute
pressure found in the first version of the library.

------------------------------------------------------------------------

## 3. The Problem with the Previous Architecture

The first version propagated pressure as an output:

``` text
element 1 output
        ↓
element 2 input
        ↓
element 2 output
        ↓
...
        ↓
return to pump
```

Conceptually, the pump performed:

``` text
P_out = P_in + ΔP_pump
```

Taken by itself, this relationship is correct. The problem came from the
iterative method: the pressure returned from the previous loop became
the new absolute pressure baseline.

For example:

``` text
start of loop      1 000 000 Pa
pump               +    1 000 Pa
losses             -      400 Pa
--------------------------------
return              1 000 600 Pa
```

The next loop then started from `1 000 600 Pa`, and the pump added its
pressure rise again.

The residual:

\[ R_P=`\Delta `{=tex}P\_{pump}-`\sum`{=tex}`\Delta `{=tex}P\_{loss} \]

was therefore implicitly being used twice:

``` text
                    pressure residual
                          │
                ┌─────────┴─────────┐
                ▼                   ▼
          increases m_dot     increases mean P
             correct            artificial
```

The second effect does not belong to the physical model of a closed,
rigid circuit containing an incompressible fluid.

------------------------------------------------------------------------

## 4. New Architecture: Pressure Belongs to Nodes

A node is now conceptually:

``` c
struct FlowNode {
    const char *name;
    double pressure;
};
```

and an element has two terminals:

``` c
FlowNode *inlet;
FlowNode *outlet;
```

The convention used by the library is:

\[ `\Delta `{=tex}P=P\_{out}-P\_{in} \]

Therefore:

``` text
pump:        ΔP > 0
pipe:        ΔP < 0
gauge:       ΔP = 0       (current ideal model)
```

A component does not determine the absolute pressure of the circuit. It
only determines the relationship between its terminals.

------------------------------------------------------------------------

## 5. The Pump

For a pump imposing a prescribed pressure rise:

\[ `\Delta `{=tex}P\_{pump}=P\_{out}-P\_{in} \]

During startup, the actual value may approach the target gradually. In
the current model:

\[ `\frac{d(\Delta P_{pump})}{dt}`{=tex} =
`\frac{\Delta P_{target}-\Delta P_{pump}}{\tau}`{=tex} \]

where (`\tau`{=tex}) is the pump time constant.

This dynamics describes **how the pump pressure rise develops**, not an
accumulation of absolute pressure.

------------------------------------------------------------------------

## 6. The Pipe and Darcy--Weisbach

A pipe is a passive element.

For a mass flow rate (`\dot `{=tex}m):

\[ Q=`\frac{\dot m}{\rho}`{=tex} \]

and, with cross-sectional area:

\[ A=`\frac{\pi D^2}{4}`{=tex} \]

the mean fluid velocity is:

\[ v=`\frac{Q}{A}`{=tex}=`\frac{\dot m}{\rho A}`{=tex} \]

The Reynolds number is:

\[ Re=`\frac{\rho |v|D}{\mu}`{=tex} \]

The distributed pressure loss is calculated using Darcy--Weisbach:

\[ `\Delta `{=tex}P\_{loss} =
f`\frac{L}{D}`{=tex}`\frac{\rho v^2}{2}`{=tex} \]

In the code, the sign is chosen so that the pressure loss opposes the
direction of motion:

\[ `\Delta `{=tex}P\_{pipe} =
-f`\frac{L}{D}`{=tex}`\frac{\rho v|v|}{2}`{=tex} \]

Therefore, as flow rate increases, pressure losses increase.

For laminar flow:

\[ f=`\frac{64}{Re}`{=tex} \]

For turbulent flow, the library uses an explicit correlation based on
relative roughness and Reynolds number.

------------------------------------------------------------------------

## 7. Why Flow Rate Increases When the Pump Starts

Consider a mass of fluid initially at rest.

A pressure difference produces a force:

\[ F=`\Delta `{=tex}P,A \]

which accelerates the fluid:

\[ F=Ma \]

When the pump starts:

``` text
ΔP_pump develops
       ↓
net force on the fluid
       ↓
velocity increases
       ↓
m_dot increases
       ↓
pipe losses increase
       ↓
pressure residual decreases
```

The fundamental point is that, during the transient, a pressure residual
**must not become an increase in mean absolute pressure**.

It must produce acceleration of the fluid.

------------------------------------------------------------------------

## 8. Lumped Dynamic Equation

In the current model, the entire serial loop has one common mass flow
rate (`\dot `{=tex}m).

Define:

\[ R_P=`\sum`{=tex}\_i `\Delta `{=tex}P_i \]

that is, for the test circuit:

\[ R_P= `\Delta `{=tex}P\_{pump} +`\Delta `{=tex}P\_{pipe1}
+`\Delta `{=tex}P\_{gauge} +`\Delta `{=tex}P\_{pipe2} \]

with:

\[ `\Delta `{=tex}P\_{gauge}=0 \]

The dynamics is written as:

\[ I_h`\frac{d\dot m}{dt}`{=tex}=R_P \]

therefore:

\[ `\frac{d\dot m}{dt}`{=tex}=`\frac{R_P}{I_h}`{=tex} \]

and numerically:

\[ `\dot `{=tex}m\_{n+1} = `\dot `{=tex}m_n+
`\frac{R_P}{I_h}`{=tex}`\Delta `{=tex}t \]

### Important Note

In the current version, (I_h), the equivalent hydraulic inertance, is
still a provisional lumped parameter.

This is the next quantity that must be made fully physical: it should
not remain an arbitrary number, but should be derived from circuit
geometry and the mass of fluid contained in the circuit.

------------------------------------------------------------------------

## 9. Absolute and Relative Pressure

For a closed, rigid circuit completely filled with an incompressible
fluid, the model requires a reference for the absolute pressure level.

In the current test:

\[ P\_{reference}=1,000,000 Pa \]

is maintained.

The pump and passive elements then determine the **relative pressure
profile** with respect to this reference.

The residual around the loop does not modify (P\_{reference}): it
modifies the flow rate.

This eliminates the previous artificial pressure drift.

------------------------------------------------------------------------

## 10. Numerical Test

Circuit:

``` text
[N0] -- PUMP -- [N1] -- PIPE 1 -- [N2] -- GAUGE -- [N3] -- PIPE 2 -- [N0]
```

Main parameters:

``` text
P_reference       = 1 000 000 Pa
ΔP_pump,target    =     1 000 Pa
L_pipe            =        10 m
D_pipe            =       0.1 m
roughness         = 0.000045 m
```

Initial condition:

\[ `\dot `{=tex}m(0)=0 \]

The pump gradually approaches:

\[ `\Delta `{=tex}P\_{pump}=1000 Pa \]

The flow rate increases and the pressure losses grow.

The long-duration test converges to approximately:

\[ `\boxed{\dot m\simeq5.42002\ kg/s}`{=tex} \]

with:

\[ `\boxed{R_P\simeq0}`{=tex} \]

and pressures approximately:

``` text
pump IN       1 000 000 Pa
pump OUT      1 001 000 Pa
pipe 1 OUT    1 000 500 Pa
gauge         1 000 500 Pa
pipe 2 OUT    1 000 000 Pa
```

Since the two pipes are identical, at steady state each dissipates
approximately:

\[ `\Delta `{=tex}P\_{pipe}`\simeq-500`{=tex} Pa \]

therefore:

\[ +1000-500-500=0 \]

------------------------------------------------------------------------

## 11. Interpretation of Steady State

Steady state is not imposed.

It emerges from the condition:

\[ R_P=0 \]

or equivalently:

\[ `\sum`{=tex}\_i`\Delta `{=tex}P_i=0 \]

In our case:

\[ `\Delta `{=tex}P\_{pump} =
`\Delta `{=tex}P\_{loss,1}(`\dot `{=tex}m) +
`\Delta `{=tex}P\_{loss,2}(`\dot `{=tex}m) \]

When this equality is satisfied:

\[ `\frac{d\dot m}{dt}`{=tex}=0 \]

and therefore:

\[ `\dot `{=tex}m=`\mathrm{constant}`{=tex} \]

This is the hydraulic operating point of the circuit.

------------------------------------------------------------------------

## 12. Complete Physical Scheme

``` text
                         PUMP
                  ΔP_pump > 0
              ┌────────────────►
              │
              │       N1
              │   P = 1.001 MPa
              │         │
              │         │ PIPE 1
              │         │ ΔP_loss < 0
              │         ▼
              │       N2
              │   P ≈ 1.0005 MPa
              │         │
              │       GAUGE
              │         │ ΔP = 0
              │         ▼
              │       N3
              │   P ≈ 1.0005 MPa
              │         │
              │         │ PIPE 2
              │         │ ΔP_loss < 0
              │         ▼
              └──────── N0
                  P = 1.000 MPa

                     m_dot →
```

At steady state:

``` text
ΔP_pump = Σ ΔP_loss
```

During the transient:

``` text
ΔP_pump - Σ ΔP_loss
            │
            ▼
      pressure residual
            │
            ▼
        acceleration
            │
            ▼
       change in m_dot
            │
            ▼
       change in losses
```

------------------------------------------------------------------------

## 13. Library Philosophy

The library deliberately tries to keep the physics visible.

### A node knows

``` text
its own pressure
```

### A pump knows

``` text
the relationship it imposes between P_in and P_out
```

### A pipe knows

``` text
its own geometry
the properties required to calculate pressure losses
how ΔP depends on flow rate
```

### The circuit knows

``` text
which elements form the flow path
the common flow rate
the pressure residual
how the residual dynamically changes the flow rate
```

This avoids assigning information to an individual component that
actually belongs to the system as a whole.

------------------------------------------------------------------------

## 14. What Is Already Physically Satisfactory

The current version has resolved the main conceptual error of the first
implementation:

1.  pressure is no longer artificially accumulated on every circulation;
2.  the pump imposes a pressure difference between its terminals;
3.  pressure losses depend on flow rate;
4.  the pressure residual accelerates or decelerates the fluid;
5.  steady state emerges from the condition
    (`\sum`{=tex}`\Delta `{=tex}P=0);
6.  absolute pressure and pressure differences are treated as separate
    concepts.

------------------------------------------------------------------------

## 15. What Is Still Simplified

The current model is intentionally minimal.

It does not yet represent:

-   fluid compressibility;
-   pipe-wall elasticity;
-   pressure-wave propagation;
-   water hammer;
-   local mass accumulation;
-   density variations;
-   energy balance;
-   heat transfer;
-   phase changes;
-   generic branches and networks;
-   valves;
-   tanks and pressurizers;
-   pumps with characteristic curves (`\Delta `{=tex}P(`\dot `{=tex}m));
-   different flow rates in different branches.

Furthermore, (I_h) is still provisional.

------------------------------------------------------------------------

## 16. Next Physical Step

The next development should not introduce additional numerical
corrections.

It should start from the momentum balance of the fluid.

For a fluid column of length (L), cross-sectional area (A), density
(`\rho`{=tex}), and mean velocity (v):

\[ M=`\rho `{=tex}A L \]

The force produced by a pressure difference is:

\[ F=`\Delta `{=tex}P,A \]

and:

\[ M`\frac{dv}{dt}`{=tex}=`\Delta `{=tex}P,A \]

therefore:

\[ `\rho `{=tex}A L`\frac{dv}{dt}`{=tex} = `\Delta `{=tex}P,A \]

which gives:

\[ `\rho `{=tex}L`\frac{dv}{dt}`{=tex}=`\Delta `{=tex}P \]

Since:

\[ `\dot `{=tex}m=`\rho `{=tex}A v \]

for an incompressible fluid with constant (`\rho`{=tex}) and (A):

\[ `\frac{d\dot m}{dt}`{=tex} = `\rho `{=tex}A`\frac{dv}{dt}`{=tex} \]

and therefore:

\[ `\Delta `{=tex}P = `\frac{L}{A}`{=tex} `\frac{d\dot m}{dt}`{=tex} \]

For a single uniform section, an inertance coefficient therefore emerges
naturally:

\[ `\boxed{I_h=\frac{L}{A}}`{=tex} \]

when the dynamic variable is mass flow rate (`\dot `{=tex}m).

For several sections in series, the equivalent inertial contribution
should be constructed from the sum of the appropriate geometric
contributions:

\[ I\_{h,tot} = `\sum`{=tex}*i I*{h,i} \]

This derivation must be checked carefully when the library is extended
to different cross-sectional areas, lumped components, and variable
fluid properties.

------------------------------------------------------------------------

## 17. Future Architectural Direction

A natural evolution is to preserve several levels of modeling.

### Level 1 --- Steady State

Solve directly:

\[ `\sum`{=tex}\_i`\Delta `{=tex}P_i(`\dot `{=tex}m)=0 \]

to obtain the operating point without simulating the transient.

### Level 2 --- Lumped Dynamic

Integrate:

\[ I_h`\frac{d\dot m}{dt}`{=tex} = `\sum`{=tex}\_i`\Delta `{=tex}P_i \]

This is the level currently under development.

### Level 3 --- Distributed Dynamic

The circuit is spatially discretized:

``` text
P0 -- cell -- P1 -- cell -- P2 -- cell -- P3
       m1            m2            m3
```

and mass, momentum, and eventually energy are solved locally.

This level will eventually make it possible to represent phenomena such
as pressure waves and fast hydraulic transients.

------------------------------------------------------------------------

## 18. Principle to Preserve

The most important rule for future development is:

> **Do not numerically correct physically incorrect behavior when the
> physical model that produces it can instead be corrected.**

The library should remain simple enough that, component by component, it
is possible to trace every computed quantity back to the equation that
produced it.

The circuit should not converge because we force it to converge.

It should converge because:

\[
`\boxed{\text{the physical equations admit that equilibrium point}}`{=tex}
\]

and the solver should do nothing more than find it.
