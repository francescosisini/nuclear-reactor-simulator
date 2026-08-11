# `flow` Library — Physical Simulation Algorithm

## 1. Purpose

The `flow` library models the hydraulic dynamics of a closed circuit containing an incompressible fluid.

The physical problem is:

> Given a set of active and passive hydraulic components, determine how the mass flow rate evolves in time and which steady operating point is eventually reached.

The simulation is based directly on the momentum balance of the fluid.

Pressure differences produce forces. These forces accelerate or decelerate the fluid. Passive hydraulic components generate pressure losses that depend on the flow rate. A steady state is reached when the total pressure rise supplied by the active components is exactly balanced by the total pressure loss of the passive components.

The numerical algorithm therefore integrates the physical equations of the circuit. It does not impose the steady state externally.

---

## 2. Physical representation of the circuit

Consider a closed serial hydraulic circuit:

```text
              m_dot
                →
[N0] -- PUMP -- [N1] -- PIPE 1 -- [N2] -- GAUGE -- [N3] -- PIPE 2 -- [N0]
 P0              P1                 P2                 P3
```

The model separates three physical quantities:

- **pressure** `P`, associated with a node;
- **mass flow rate** `\dot{m}`, associated with the flow through the circuit;
- **pressure difference** `\Delta P`, associated with a hydraulic component.

Each component connects an inlet node to an outlet node.

The sign convention is

```math
\Delta P = P_{out} - P_{in}
```

An active component such as a pump normally produces

```math
\Delta P_{pump} > 0
```

A passive component carrying flow in the positive direction produces

```math
\Delta P_{passive} < 0
```

An ideal pressure gauge produces no hydraulic loss:

```math
\Delta P_{gauge} = 0
```

The component therefore does not own an absolute pressure. It defines a relationship between the pressures at its two terminals.

---

## 3. Governing equation of the loop

For a closed hydraulic loop, define the total pressure residual

```math
R_P = \sum_i \Delta P_i
```

For the example circuit,

```math
R_P =
\Delta P_{pump}
+
\Delta P_{pipe1}
+
\Delta P_{gauge}
+
\Delta P_{pipe2}
```

Since the gauge is ideal,

```math
\Delta P_{gauge}=0
```

and therefore

```math
R_P =
\Delta P_{pump}
+
\Delta P_{pipe1}
+
\Delta P_{pipe2}
```

The residual has a direct physical meaning.

If

```math
R_P > 0
```

there is a net pressure force in the positive direction and the fluid accelerates.

If

```math
R_P < 0
```

the net force opposes the current direction of motion and the fluid decelerates.

If

```math
R_P = 0
```

the hydraulic forces are balanced and the mass flow rate no longer changes.

The lumped momentum equation of the circuit is

```math
I_h \frac{d\dot{m}}{dt} = R_P
```

or

```math
\frac{d\dot{m}}{dt} = \frac{R_P}{I_h}
```

where `I_h` is the equivalent hydraulic inertance of the moving fluid.

This equation is the core of the dynamic simulation.

---

## 4. Derivation of hydraulic inertance

The hydraulic inertance follows directly from Newton's second law.

Consider a uniform column of incompressible fluid with:

- length `L`;
- cross-sectional area `A`;
- density `\rho`;
- mean velocity `v`.

The mass of fluid contained in the section is

```math
M = \rho A L
```

A pressure difference `\Delta P` across the section produces a force

```math
F = \Delta P A
```

Newton's second law gives

```math
M\frac{dv}{dt}=F
```

Substituting the expressions for mass and pressure force,

```math
\rho A L\frac{dv}{dt}=\Delta P A
```

and therefore

```math
\rho L\frac{dv}{dt}=\Delta P
```

For an incompressible fluid,

```math
\dot{m}=\rho A v
```

If `\rho` and `A` are constant,

```math
\frac{d\dot{m}}{dt}
=
\rho A\frac{dv}{dt}
```

so

```math
\frac{dv}{dt}
=
\frac{1}{\rho A}\frac{d\dot{m}}{dt}
```

Substituting into the momentum equation gives

```math
\Delta P
=
\frac{L}{A}\frac{d\dot{m}}{dt}
```

The hydraulic inertance of a uniform section is therefore

```math
I_h=\frac{L}{A}
```

when the dynamic variable is the mass flow rate `\dot{m}`.

For several sections in series carrying the same mass flow rate,

```math
I_{h,tot}
=
\sum_i I_{h,i}
```

and therefore

```math
I_{h,tot}
=
\sum_i \frac{L_i}{A_i}
```

The momentum equation for the complete serial circuit becomes

```math
\left(\sum_i \frac{L_i}{A_i}\right)
\frac{d\dot{m}}{dt}
=
\sum_i \Delta P_i
```

---

## 5. Pump model

A pressure-driven pump imposes a pressure rise between its inlet and outlet:

```math
\Delta P_{pump}
=
P_{out}-P_{in}
```

The pump therefore behaves as an active two-terminal hydraulic component.

For an ideal pump with a fixed pressure rise,

```math
\Delta P_{pump}
=
\Delta P_{target}
```

If startup dynamics are included, the pressure rise can approach its target progressively:

```math
\frac{d\Delta P_{pump}}{dt}
=
\frac{\Delta P_{target}-\Delta P_{pump}}{\tau}
```

where `\tau` is the pump time constant.

Using an explicit time step,

```math
\Delta P_{pump,n+1}
=
\Delta P_{pump,n}
+
\frac{\Delta P_{target}-\Delta P_{pump,n}}{\tau}\Delta t
```

The pump pressure rise is a difference between two node pressures. It is not an increment added repeatedly to the absolute pressure of the circuit.

---

## 6. Pipe model

A pipe is a passive component whose pressure drop depends on the current flow rate and on its physical geometry.

For a circular pipe of diameter `D`,

```math
A=\frac{\pi D^2}{4}
```

The volumetric flow rate is

```math
Q=\frac{\dot{m}}{\rho}
```

and the mean fluid velocity is

```math
v=\frac{Q}{A}
```

or

```math
v=\frac{\dot{m}}{\rho A}
```

The Reynolds number is

```math
Re=\frac{\rho |v|D}{\mu}
```

where `\mu` is the dynamic viscosity.

The Darcy-Weisbach relation gives the magnitude of the distributed pressure loss:

```math
\Delta P_{loss}
=
f\frac{L}{D}\frac{\rho v^2}{2}
```

where `f` is the Darcy friction factor.

To include the direction of the force explicitly, the pipe pressure difference is written as

```math
\Delta P_{pipe}
=
-f\frac{L}{D}\frac{\rho v|v|}{2}
```

The term `v|v|` ensures that the pressure force generated by friction always opposes the direction of motion.

For laminar flow,

```math
f=\frac{64}{Re}
```

For turbulent flow, `f` is calculated from Reynolds number and relative roughness using the correlation implemented by the library.

The calculation performed by a pipe is therefore

```text
m_dot
  ↓
velocity
  ↓
Reynolds number
  ↓
friction factor
  ↓
pressure drop
```

Whenever `\dot{m}` changes, the pipe pressure loss is recalculated.

---

## 7. Evaluation of the pressure residual

Once the pressure contribution of every component has been calculated, the circuit evaluates

```math
R_P=\sum_i \Delta P_i
```

For a pump followed by two passive pipes,

```math
R_P
=
\Delta P_{pump}
+
\Delta P_{pipe1}
+
\Delta P_{pipe2}
```

Because pipe pressure differences are negative in the direction of positive flow, this can also be written in terms of positive loss magnitudes:

```math
R_P
=
\Delta P_{pump}
-
\Delta P_{loss,1}
-
\Delta P_{loss,2}
```

The residual is the net pressure force available to accelerate the fluid.

It is therefore the hydraulic equivalent of the resultant force in Newton's equation.

---

## 8. Dynamic update of the mass flow rate

The momentum equation is

```math
I_h\frac{d\dot{m}}{dt}=R_P
```

so

```math
\frac{d\dot{m}}{dt}
=
\frac{R_P}{I_h}
```

With an explicit Euler integration step,

```math
\dot{m}_{n+1}
=
\dot{m}_n
+
\frac{R_{P,n}}{I_h}\Delta t
```

This is the fundamental numerical update of the hydraulic state.

The numerical solver does not directly change the pressure to force convergence.

It changes the mass flow rate according to the acceleration produced by the current pressure imbalance.

---

## 9. Reconstruction of node pressures

The dynamic equation determines the evolution of `\dot{m}`.

The node pressures can then be reconstructed from the component pressure differences.

Because only pressure differences enter the incompressible hydraulic momentum balance, one absolute pressure must be chosen as a reference.

Let

```math
P_0=P_{reference}
```

Starting from this node, the pressure after the pump is

```math
P_1
=
P_0+\Delta P_{pump}
```

After the first pipe,

```math
P_2
=
P_1+\Delta P_{pipe1}
```

After an ideal gauge,

```math
P_3
=
P_2+\Delta P_{gauge}
```

and after the final pipe,

```math
P_0^{calc}
=
P_3+\Delta P_{pipe2}
```

At hydraulic equilibrium,

```math
P_0^{calc}=P_0
```

because

```math
\sum_i \Delta P_i=0
```

During a transient,

```math
P_0^{calc}-P_0=R_P
```

represents the instantaneous unbalanced pressure force around the lumped loop.

That residual is used in the momentum equation to accelerate the fluid. It is not used to redefine the absolute pressure reference.

---

## 10. One complete simulation step

Assume that at time `t_n` the circuit has mass flow rate

```math
\dot{m}_n
```

The simulation advances to `t_{n+1}` as follows.

### 10.1 Update the pump

Calculate the pressure rise currently produced by the pump:

```math
\Delta P_{pump,n}
```

If pump startup dynamics are enabled, update the internal pump state before evaluating the loop.

### 10.2 Evaluate every passive component

For each pipe, use `\dot{m}_n` to calculate:

```math
v_n
```

then

```math
Re_n
```

then

```math
f_n
```

and finally

```math
\Delta P_{pipe,n}
```

### 10.3 Sum all pressure differences

Calculate

```math
R_{P,n}
=
\sum_i \Delta P_{i,n}
```

### 10.4 Calculate the flow acceleration

Apply

```math
\frac{d\dot{m}}{dt}\bigg|_n
=
\frac{R_{P,n}}{I_h}
```

### 10.5 Advance the flow rate

Integrate over `\Delta t`:

```math
\dot{m}_{n+1}
=
\dot{m}_n
+
\frac{R_{P,n}}{I_h}\Delta t
```

### 10.6 Reconstruct node pressures

Set

```math
P_0=P_{reference}
```

and propagate the component pressure differences once around the circuit to obtain `P_1`, `P_2`, `P_3`, and the other node pressures.

The new mass flow rate then becomes the input to the next simulation step.

---

## 11. Complete dynamic algorithm

The complete feedback loop is

```text
current mass flow rate
        │
        ▼
evaluate pump and passive components
        │
        ▼
calculate every ΔP_i
        │
        ▼
R_P = Σ ΔP_i
        │
        ▼
d(m_dot)/dt = R_P / I_h
        │
        ▼
integrate over Δt
        │
        ▼
new mass flow rate
        │
        └──────────────► next iteration
```

At each step, node pressures are reconstructed separately:

```text
P_reference
     │
     ▼
   pump
     │
     ▼
    P1
     │
   pipe 1
     │
     ▼
    P2
     │
   gauge
     │
     ▼
    P3
     │
   pipe 2
     │
     ▼
P_reference
```

At steady state, the pressure profile closes exactly after one complete loop.

---

## 12. Physical evolution from rest

Suppose the fluid is initially stationary:

```math
\dot{m}(0)=0
```

At zero flow, friction losses are zero or very small.

When the pump develops a positive pressure rise,

```math
R_P>0
```

and therefore

```math
\frac{d\dot{m}}{dt}>0
```

The fluid begins to accelerate.

As the mass flow rate increases,

```math
|\Delta P_{pipe}|
```

also increases.

The total passive pressure loss therefore grows, reducing the residual:

```math
R_P
=
\Delta P_{pump}
-
\sum_i \Delta P_{loss,i}
```

As `R_P` becomes smaller, the acceleration also becomes smaller.

Eventually,

```math
R_P=0
```

and therefore

```math
\frac{d\dot{m}}{dt}=0
```

The mass flow rate then remains constant.

The steady operating point is thus generated automatically by the balance between the pump and the hydraulic resistance of the circuit.

---

## 13. Steady-state equation

At steady state,

```math
\frac{d\dot{m}}{dt}=0
```

so the momentum equation becomes

```math
\sum_i \Delta P_i=0
```

For the simple circuit,

```math
\Delta P_{pump}
+
\Delta P_{pipe1}(\dot{m})
+
\Delta P_{pipe2}(\dot{m})
=
0
```

or equivalently,

```math
\Delta P_{pump}
=
\Delta P_{loss,1}(\dot{m})
+
\Delta P_{loss,2}(\dot{m})
```

The operating mass flow rate `\dot{m}^*` is the value that satisfies this nonlinear equation.

Define

```math
F(\dot{m})
=
\sum_i \Delta P_i(\dot{m})
```

The steady-state problem is simply

```math
F(\dot{m}^*)=0
```

This is a root-finding problem.

A dedicated steady-state solver can therefore use bisection, the secant method, Newton's method, or another root-finding algorithm without changing the physical models of the pump and pipes.

---

## 14. Dynamic solver and steady-state solver

The same component physics supports two different numerical problems.

### Dynamic simulation

Solve the differential equation

```math
I_h\frac{d\dot{m}}{dt}
=
\sum_i \Delta P_i
```

to obtain

```math
\dot{m}(t)
```

This describes startup, shutdown, and other transients represented by the lumped model.

### Steady-state simulation

Solve directly

```math
\sum_i \Delta P_i(\dot{m})=0
```

to obtain the operating point without simulating the transient.

The physical component equations are identical in both cases.

Only the numerical solver changes.

---

## 15. Example equilibrium

Consider a circuit with

```text
P_reference       = 1 000 000 Pa
ΔP_pump           =     1 000 Pa
L_pipe            =        10 m
D_pipe            =       0.1 m
roughness         = 0.000045 m
```

and two identical pipes.

The equilibrium mass flow rate obtained in the current test is approximately

```math
\dot{m}
\simeq
5.42002\ kg/s
```

At this operating point, each identical pipe dissipates approximately

```math
\Delta P_{pipe}
\simeq
-500\ Pa
```

The loop balance is therefore

```math
1000-500-500=0
```

and hence

```math
R_P=0
```

The pressure profile is approximately

```text
pump inlet       1 000 000 Pa
pump outlet      1 001 000 Pa
pipe 1 outlet    1 000 500 Pa
gauge            1 000 500 Pa
pipe 2 outlet    1 000 000 Pa
```

The pressure returns to its reference value after one complete circuit because the sum of all component pressure differences is zero.

---

## 16. Physical responsibilities of the library objects

The software architecture follows the physical decomposition of the equations.

### Node

A node stores its pressure:

```text
FlowNode
    pressure
```

### Pump

A pump defines the pressure rise between its terminals:

```math
\Delta P_{pump}
=
P_{out}-P_{in}
```

### Pipe

A pipe stores the physical information required to evaluate its own hydraulic resistance:

```text
length
diameter
roughness
fluid properties
```

and calculates

```math
\Delta P_{pipe}
=
f(\dot{m},L,D,\epsilon,\rho,\mu)
```

### Ideal gauge

The current ideal gauge reads pressure without affecting the hydraulic circuit:

```math
\Delta P_{gauge}=0
```

### Circuit

The circuit owns quantities that describe the hydraulic system as a whole:

```text
mass flow rate
hydraulic inertance
pressure residual
time step
time integration
```

This separation ensures that a component only contains information associated with its physical role.

---

## 17. Minimal pseudocode

A minimal dynamic solver can be expressed as:

```text
initialize nodes
initialize components
initialize m_dot
initialize pump state

calculate I_h

for each time step:

    update pump state

    for each component:
        calculate ΔP_i using current m_dot

    R_P = sum(ΔP_i)

    dm_dot_dt = R_P / I_h

    m_dot = m_dot + dm_dot_dt * dt

    set reference pressure

    reconstruct node pressures from ΔP_i

    record the state
```

The physical feedback is

```text
pressure imbalance
        ↓
fluid acceleration
        ↓
change in mass flow rate
        ↓
change in hydraulic losses
        ↓
change in pressure imbalance
```

---

## 18. Physical equilibrium and numerical convergence

Physical equilibrium and numerical convergence are different concepts.

The physical equilibrium condition is

```math
R_P=0
```

or

```math
\sum_i \Delta P_i=0
```

This implies

```math
\frac{d\dot{m}}{dt}=0
```

Numerical convergence means that the discrete solution is sufficiently close to this physical state.

For example, the solver may stop when

```math
|R_P|<\varepsilon_P
```

and

```math
|\dot{m}_{n+1}-\dot{m}_n|
<
\varepsilon_{\dot{m}}
```

The tolerances `\varepsilon_P` and `\varepsilon_{\dot{m}}` determine the numerical accuracy required by the simulation.

They do not determine the physical operating point.

---

## 19. Numerical integration

The current explicit Euler update is

```math
\dot{m}_{n+1}
=
\dot{m}_n
+
\Delta t
\frac{R_{P,n}}{I_h}
```

This is the simplest time integration of the governing differential equation.

The physical model can be written abstractly as

```math
\frac{d\dot{m}}{dt}
=
G(\dot{m},t)
```

with

```math
G(\dot{m},t)
=
\frac{1}{I_h}
\sum_i \Delta P_i(\dot{m},t)
```

A different numerical integrator can therefore replace Euler without changing the hydraulic component models.

For example, higher-order Runge-Kutta methods can integrate the same function `G`.

The architecture should preserve the separation

```text
component physics
       ↓
governing differential equation
       ↓
numerical integration method
       ↓
time evolution
```

---

## 20. Current model assumptions

The algorithm currently assumes:

- an incompressible fluid;
- a rigid hydraulic circuit;
- a single serial flow path;
- the same mass flow rate through every component;
- lumped hydraulic inertia;
- quasi-steady friction losses;
- constant fluid properties in the basic model;
- no local mass accumulation;
- no heat transfer;
- no phase transition;
- no pressure-wave propagation.

Under these assumptions, the complete hydraulic dynamics can be represented by one main state variable:

```math
\dot{m}(t)
```

together with any internal dynamic states of active components, such as the pump startup state.

---

## 21. Extension to hydraulic networks

A branched network requires more state variables, but the physical principles remain the same.

At each node, mass conservation requires

```math
\sum_j \dot{m}_j=0
```

with the signs chosen according to the direction of each connected branch.

Each branch has its own momentum relation:

```math
I_{h,j}\frac{d\dot{m}_j}{dt}
=
\Delta P_j
-
\Delta P_{loss,j}
```

and each component still supplies a relation of the form

```math
\Delta P_i
=
f_i(\dot{m}_i,\text{component parameters})
```

Node pressures couple the branch equations.

The present serial-loop model is therefore the simplest case of a more general nodal hydraulic formulation.

---

## 22. Core simulation principle

The physical algorithm can be summarized by Newton's second law applied to the fluid:

```math
\text{net pressure force}
=
\text{hydraulic inertia}
\times
\text{flow acceleration}
```

or, for the lumped loop,

```math
\boxed{
I_h\frac{d\dot{m}}{dt}
=
\sum_i \Delta P_i
}
```

The pump contributes positive pressure rise.

Passive components contribute pressure drops.

The resulting imbalance changes the flow rate.

The new flow rate changes the passive losses.

The process is repeated until

```math
\boxed{
\sum_i \Delta P_i=0
}
```

At that point,

```math
\boxed{
\frac{d\dot{m}}{dt}=0
}
```

and the circuit has reached its hydraulic operating point.

The numerical solver therefore performs only three fundamental operations:

```text
1. evaluate the hydraulic forces;
2. calculate the resulting acceleration;
3. integrate the state in time.
```

Everything else follows from these physical equations.
