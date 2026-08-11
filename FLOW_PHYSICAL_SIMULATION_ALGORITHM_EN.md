# `flow` Library — Physical Simulation Algorithm

## 1. Purpose

The `flow` library models the hydraulic dynamics of a closed circuit containing an incompressible fluid.

The physical problem is:

> Given a set of active and passive hydraulic components, determine how the volumetric flow rate evolves in time and which steady operating point is eventually reached.

The simulation is based directly on the momentum balance of the fluid.

Pressure differences produce forces. These forces accelerate or decelerate the fluid. Passive hydraulic components generate pressure losses that depend on the flow rate. A steady state is reached when the total pressure rise supplied by the active components is exactly balanced by the total pressure loss of the passive components.

The numerical algorithm therefore integrates the physical equations of the circuit. It does not impose the steady state externally.

---

## 2. Physical representation of the circuit

Consider a closed serial hydraulic circuit:

```text
              Q
                →
[N0] -- PUMP -- [N1] -- PIPE 1 -- [N2] -- GAUGE -- [N3] -- PIPE 2 -- [N0]
 P0              P1                 P2                 P3
```

The model separates three physical quantities:

- **pressure** `P`, associated with a node;
- **volumetric flow rate** `Q`, associated with the flow through the circuit;
- **mass flow rate** `\dot{m}=\rho Q`, derived from `Q` when needed;
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

For a closed loop, the sum of the actual node-to-node pressure changes around one complete circuit is always zero:

```math
\oint_{\mathcal C} dP = 0
```

Equivalently, if the loop is represented by discrete components,

```math
\sum_i \Delta P_i^{terminal}=0
```

where

```math
\Delta P_i^{terminal}=P_{out,i}-P_{in,i}
```

is the actual pressure difference between the terminals of component `i`.

The quantity that accelerates the fluid is therefore not the circulation of the pressure field itself. It is the part of the pressure balance that remains after dissipative losses are subtracted from the pressure rise supplied by active components.

For the example circuit, define the **driving pressure residual**

```math
R_P
=
\Delta P_{pump}
-
\Delta P_{loss,1}
-
\Delta P_{loss,2}
```

If

```math
R_P>0
```

the pump supplies more pressure rise than is currently dissipated by friction, so the fluid accelerates.

If

```math
R_P<0
```

the dissipative losses exceed the active pressure rise, so the fluid decelerates.

If

```math
R_P=0
```

the hydraulic forces are balanced and the volumetric flow rate no longer changes.

The lumped momentum equation of the complete loop is

```math
I_h\frac{dQ}{dt}=R_P
```

or

```math
\frac{dQ}{dt}
=
\frac{R_P}{I_h}
```

Thus the global dynamic balance is

```math
\Delta P_{active}
-
\Delta P_{dissipative}
=
\Delta P_{inertial}
```

with

```math
\Delta P_{inertial}
=
I_h\frac{dQ}{dt}
```

This is the core of the dynamic simulation.

---

## 4. Derivation of hydraulic inertance

The hydraulic inertance follows directly from Newton's second law.

Consider a uniform section of incompressible fluid with:

- length `L`;
- cross-sectional area `A`;
- density `\rho`;
- mean velocity `v`.

The mass of fluid contained in the section is

```math
M=\rho A L
```

A net pressure-force imbalance acting on that fluid section is

```math
F=\Delta P_{inertial}A
```

Newton's second law gives

```math
M\frac{dv}{dt}=F
```

therefore

```math
\rho A L\frac{dv}{dt}
=
\Delta P_{inertial}A
```

and

```math
\rho L\frac{dv}{dt}
=
\Delta P_{inertial}
```

For an incompressible fluid,

```math
Q=Av
```

and, if `A` is constant,

```math
\frac{dQ}{dt}
=
A\frac{dv}{dt}
```

so

```math
\frac{dv}{dt}
=
\frac{1}{A}\frac{dQ}{dt}
```

Substituting gives

```math
\Delta P_{inertial}
=
\frac{\rho L}{A}\frac{dQ}{dt}
```

The hydraulic inertance of the section is therefore

```math
I_h
=
\frac{\rho L}{A}
```

so that

```math
\Delta P_{inertial}
=
I_h\frac{dQ}{dt}
```

This equation describes the **inertial contribution** to the pressure balance. It is not, by itself, the complete pressure drop of a real pipe, because a real pipe also dissipates pressure through friction.

For several sections in series carrying the same volumetric flow rate,

```math
I_{h,tot}
=
\sum_i I_{h,i}
```

and therefore

```math
I_{h,tot}
=
\sum_i \frac{\rho_iL_i}{A_i}
```

The global momentum equation becomes

```math
\left(
\sum_i \frac{\rho_iL_i}{A_i}
\right)
\frac{dQ}{dt}
=
\Delta P_{active}
-
\sum_i \Delta P_{loss,i}
```

For a loop with one pump,

```math
\left(
\sum_i \frac{\rho_iL_i}{A_i}
\right)
\frac{dQ}{dt}
=
\Delta P_{pump}
-
\sum_i \Delta P_{loss,i}
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

A real pipe contributes two physically different terms to the momentum balance:

1. an **inertial term**, associated with acceleration of the fluid contained in the pipe;
2. a **dissipative term**, associated with wall friction.

For a circular pipe of diameter `D`,

```math
A=\frac{\pi D^2}{4}
```

The mean fluid velocity is

```math
v=\frac{Q}{A}
```

The Reynolds number is

```math
Re=\frac{\rho|v|D}{\mu}
```

where `\mu` is the dynamic viscosity.

The Darcy-Weisbach relation gives the magnitude of the distributed friction loss:

```math
\Delta P_{loss}
=
f\frac{L}{D}\frac{\rho v^2}{2}
```

For a signed formulation that opposes the direction of flow,

```math
\Delta P_{friction}
=
-f\frac{L}{D}\frac{\rho v|v|}{2}
```

For laminar flow,

```math
f=\frac{64}{Re}
```

For turbulent flow, `f` is calculated from Reynolds number and relative roughness using the correlation implemented by the library.

During a transient, the total pressure drop from the pipe inlet to its outlet is

```math
P_{in}-P_{out}
=
\frac{\rho L}{A}\frac{dQ}{dt}
+
\Delta P_{loss}(Q)
```

or, using the terminal sign convention,

```math
P_{out}-P_{in}
=
-
\frac{\rho L}{A}\frac{dQ}{dt}
-
\Delta P_{loss}(Q)
```

At steady state,

```math
\frac{dQ}{dt}=0
```

and the relation reduces to

```math
P_{in}-P_{out}
=
\Delta P_{loss}(Q)
```

Therefore a pipe can have a finite pressure drop even when the flow rate is constant.

The friction part of the pipe calculation is

```text
Q
  ↓
velocity
  ↓
Reynolds number
  ↓
friction factor
  ↓
friction pressure loss
```

The inertial contribution is determined by the common loop acceleration `dQ/dt` and the pipe inertance `\rho L/A`.

---

## 7. Evaluation of the driving pressure residual

The circuit first evaluates the active pressure rise and the dissipative pressure losses.

For a pump followed by two passive pipes,

```math
R_P
=
\Delta P_{pump}
-
\Delta P_{loss,1}
-
\Delta P_{loss,2}
```

This quantity is not the sum of the actual terminal pressure differences around the closed loop. That sum is always zero when the loop is closed consistently.

Instead,

```math
R_P
=
\Delta P_{inertial,tot}
```

and therefore

```math
R_P
=
I_{h,tot}\frac{dQ}{dt}
```

For several serial fluid sections,

```math
R_P
=
\left(
\sum_i \frac{\rho_iL_i}{A_i}
\right)
\frac{dQ}{dt}
```

The residual is therefore the hydraulic analogue of the resultant force in Newton's second law.

---

## 8. Dynamic update of the volumetric flow rate

The momentum equation is

```math
I_h\frac{dQ}{dt}=R_P
```

so

```math
\frac{dQ}{dt}
=
\frac{R_P}{I_h}
```

With an explicit Euler integration step,

```math
Q_{n+1}
=
Q_n
+
\frac{R_{P,n}}{I_h}\Delta t
```

This is the fundamental numerical update of the hydraulic state.

The numerical solver does not directly change the pressure to force convergence.

It changes the volumetric flow rate according to the acceleration produced by the current pressure imbalance.

---

## 9. Reconstruction of node pressures

The dynamic equation determines the evolution of `Q`.

Once `dQ/dt` is known, the actual node-to-node pressure differences can be reconstructed.

Because only pressure differences enter the incompressible momentum balance, one absolute pressure must be chosen as a reference.

Let

```math
P_0=P_{reference}
```

After the pump,

```math
P_1
=
P_0+\Delta P_{pump}
```

For pipe 1, the transient pressure drop is

```math
P_1-P_2
=
I_{h,1}\frac{dQ}{dt}
+
\Delta P_{loss,1}(Q)
```

so

```math
P_2
=
P_1
-
I_{h,1}\frac{dQ}{dt}
-
\Delta P_{loss,1}(Q)
```

For an ideal gauge,

```math
P_3=P_2
```

For pipe 2,

```math
P_3-P_0
=
I_{h,2}\frac{dQ}{dt}
+
\Delta P_{loss,2}(Q)
```

Summing all component relations around the loop gives

```math
\Delta P_{pump}
-
\Delta P_{loss,1}
-
\Delta P_{loss,2}
-
\left(
I_{h,1}+I_{h,2}
\right)
\frac{dQ}{dt}
=
0
```

which is exactly the global momentum equation.

The pressure profile therefore closes at every instant when both inertial and dissipative contributions are included consistently.

At steady state,

```math
\frac{dQ}{dt}=0
```

and only the friction losses remain in the pipe pressure drops.

---

## 10. One complete simulation step

Assume that at time `t_n` the circuit has volumetric flow rate

```math
Q_n
```

The simulation advances to `t_{n+1}` as follows.

### 10.1 Update the pump

Calculate the pressure rise currently produced by the pump:

```math
\Delta P_{pump,n}
```

If pump startup dynamics are enabled, update the internal pump state before evaluating the loop.

### 10.2 Evaluate every passive component

For each pipe, use `Q_n` to calculate:

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

### 10.3 Calculate the driving pressure residual

Calculate the active pressure rise minus the dissipative losses:

```math
R_{P,n}
=
\Delta P_{active,n}
-
\sum_i \Delta P_{loss,i,n}
```

For a single-pump loop,

```math
R_{P,n}
=
\Delta P_{pump,n}
-
\sum_i \Delta P_{loss,i,n}
```

### 10.4 Calculate the rate-of-change of volumetric flow

Apply

```math
\frac{dQ}{dt}\bigg|_n
=
\frac{R_{P,n}}{I_h}
```

### 10.5 Advance the flow rate

Integrate over `\Delta t`:

```math
Q_{n+1}
=
Q_n
+
\frac{R_{P,n}}{I_h}\Delta t
```

### 10.6 Reconstruct node pressures

Set

```math
P_0=P_{reference}
```

and reconstruct the node pressures using the full terminal relation of each component. For a pipe, this includes both the inertial term `I_{h,i} dQ/dt` and the friction loss `\Delta P_{loss,i}`.

The new volumetric flow rate then becomes the input to the next simulation step.

---

## 11. Complete dynamic algorithm

The complete feedback loop is

```text
current volumetric flow rate
        │
        ▼
evaluate pump and passive components
        │
        ▼
calculate pump rise and friction losses
        │
        ▼
R_P = ΔP_active - Σ ΔP_loss
        │
        ▼
d(Q)/dt = R_P / I_h
        │
        ▼
integrate over Δt
        │
        ▼
new volumetric flow rate
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
Q(0)=0
```

At zero flow, friction losses are zero or very small.

When the pump develops a positive pressure rise,

```math
R_P>0
```

and therefore

```math
\frac{dQ}{dt}>0
```

The fluid begins to accelerate.

As the volumetric flow rate increases,

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
\frac{dQ}{dt}=0
```

The volumetric flow rate then remains constant.

The steady operating point is thus generated automatically by the balance between the pump and the hydraulic resistance of the circuit.

---

## 13. Steady-state equation

At steady state,

```math
\frac{dQ}{dt}=0
```

so the inertial term vanishes and the loop balance becomes

```math
\Delta P_{active}
=
\sum_i \Delta P_{loss,i}
```

For the simple circuit,

```math
\Delta P_{pump}
+
\Delta P_{pipe1}(Q)
+
\Delta P_{pipe2}(Q)
=
0
```

or equivalently,

```math
\Delta P_{pump}
=
\Delta P_{loss,1}(Q)
+
\Delta P_{loss,2}(Q)
```

The operating volumetric flow rate `Q^*` is the value that satisfies this nonlinear equation.

Define

```math
F(Q)
=
\Delta P_{active}(Q)
-
\sum_i \Delta P_{loss,i}(Q)
```

The steady-state problem is simply

```math
F(Q^*)=0
```

This is a root-finding problem.

A dedicated steady-state solver can therefore use bisection, the secant method, Newton's method, or another root-finding algorithm without changing the physical models of the pump and pipes.

---

## 14. Dynamic solver and steady-state solver

The same component physics supports two different numerical problems.

### Dynamic simulation

Solve the differential equation

```math
I_h\frac{dQ}{dt}
=
\sum_i \Delta P_i
```

to obtain

```math
Q(t)
```

This describes startup, shutdown, and other transients represented by the lumped model.

### Steady-state simulation

Solve directly

```math
\Delta P_{active}(Q)
-
\sum_i \Delta P_{loss,i}(Q)
=
0
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

The equilibrium volumetric flow rate obtained in the current test is approximately

```math
Q
\simeq
5.42002\times10^{-3}\ m^3/s
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
f(Q,L,D,\epsilon,\rho,\mu)
```

### Ideal gauge

The current ideal gauge reads pressure without affecting the hydraulic circuit:

```math
\Delta P_{gauge}=0
```

### Circuit

The circuit owns quantities that describe the hydraulic system as a whole:

```text
volumetric flow rate
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
initialize Q
initialize pump state

calculate I_h

for each time step:

    update pump state

    calculate active pressure rise

    for each passive component:
        calculate friction loss using current Q

    R_P = active pressure rise - total friction loss

    dQ_dt = R_P / I_h

    Q = Q + dQ_dt * dt

    set reference pressure

    reconstruct node pressures from inertial and dissipative terms

    record the state
```

The physical feedback is

```text
pressure imbalance
        ↓
fluid acceleration
        ↓
change in volumetric flow rate
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

or, equivalently,

```math
\Delta P_{active}
=
\sum_i \Delta P_{loss,i}
```

This implies

```math
\frac{dQ}{dt}=0
```

Numerical convergence means that the discrete solution is sufficiently close to this physical state.

For example, the solver may stop when

```math
|R_P|<\varepsilon_P
```

and

```math
|Q_{n+1}-Q_n|
<
\varepsilon_{Q}
```

The tolerances `\varepsilon_P` and `\varepsilon_{Q}` determine the numerical accuracy required by the simulation.

They do not determine the physical operating point.

---

## 19. Numerical integration

The current explicit Euler update is

```math
Q_{n+1}
=
Q_n
+
\Delta t
\frac{R_{P,n}}{I_h}
```

This is the simplest time integration of the governing differential equation.

The physical model can be written abstractly as

```math
\frac{dQ}{dt}
=
G(Q,t)
```

with

```math
G(Q,t)
=
\frac{1}{I_h}
\sum_i \Delta P_i(Q,t)
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
- the same volumetric flow rate through every component;
- lumped hydraulic inertia;
- quasi-steady friction losses;
- constant fluid properties in the basic model;
- no local mass accumulation;
- no heat transfer;
- no phase transition;
- no pressure-wave propagation.

Under these assumptions, the complete hydraulic dynamics can be represented by one main state variable:

```math
Q(t)
```

together with any internal dynamic states of active components, such as the pump startup state.

---

## 21. Extension to hydraulic networks

A branched network requires more state variables, but the physical principles remain the same.

At each node, mass conservation requires

```math
\sum_j Q_j=0
```

with the signs chosen according to the direction of each connected branch.

Each branch has its own momentum relation:

```math
I_{h,j}\frac{dQ_j}{dt}
=
\Delta P_j
-
\Delta P_{loss,j}
```

and each component still supplies a relation of the form

```math
\Delta P_i
=
f_i(Q_i,\text{component parameters})
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
\text{rate-of-change of volumetric flow}
```

or, for the lumped loop,

```math
\boxed{
I_h\frac{dQ}{dt}
=
\sum_i \Delta P_i
}
```

The pump contributes positive pressure rise.

Passive components contribute pressure drops.

The active-minus-dissipative pressure imbalance changes the flow rate.

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
\frac{dQ}{dt}=0
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
