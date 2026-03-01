# Building a 2D Physics Engine from Scratch — A 4-Part Dev Blog

*A technical deep-dive into UMBRA Engine's `PhysicsService`: from Newton's laws to sleeping bodies.*

---

## Part 1 — Kinematic Integration: Teaching Bodies to Move

Every physics engine begins with the same question: *given the forces acting on a body right now, where will it be next frame?* Answering that question requires translating continuous, real-world physics into discrete simulation steps. Before we touch any code, let's build the mathematical foundation from first principles.

---

### Motion in Continuous Time: Derivatives

In classical mechanics, the motion of a body is described by a hierarchy of quantities, each being the time-derivative of the one below it.

**Position** `x(t)` tells us where a body is at time `t`.

**Velocity** is the rate of change of position — how fast and in what direction the body moves:

```
v(t) = dx/dt
```

This is a *differential*: it describes an instantaneous relationship. At any instant, velocity is the slope of the position curve. A ball at `x = 3m` with `v = 5 m/s` will be near `x = 3.05m` one hundredth of a second later — but "near" is doing a lot of work, because velocity itself can change.

**Acceleration** is the rate of change of velocity:

```
a(t) = dv/dt = d²x/dt²
```

Acceleration tells us how velocity evolves. A constant acceleration of `a = 10 m/s²` means velocity grows by 10 m/s every second. These two differential relationships — `v = dx/dt` and `a = dv/dt` — are the entire kinematic foundation. Everything else follows from deciding *how* to step forward in time.

#### The Angular Equivalents

Every linear quantity has a rotational counterpart:

| Linear | Angular | Relationship |
|--------|---------|-------------|
| Position `x` | Angle `θ` | `ω = dθ/dt` |
| Velocity `v` | Angular velocity `ω` | `α = dω/dt` |
| Acceleration `a` | Angular acceleration `α` | `α = d²θ/dt²` |

A spinning box has an angular velocity `ω` (radians per second) and an angular acceleration `α` that changes it. The math is identical — we just swap vectors for scalars (in 2D, rotation is around a single axis).

---

### From Differentials to Integration

A physics engine doesn't have access to continuous time. It advances in discrete steps of size `dt` (UMBRA uses a fixed `dt = 0.01s` by default). The question becomes: given `v = dx/dt`, how do we compute the *new* position from the *current* state?

The answer is **numerical integration** — the reverse of differentiation. We want to solve:

```
x(t + dt) = x(t) + ∫[t to t+dt] v(τ) dτ
v(t + dt) = v(t) + ∫[t to t+dt] a(τ) dτ
```

In the continuous world, these integrals give exact results. But we can't evaluate a continuous integral in a simulation — we only know `v` and `a` at specific moments. Different approximation strategies give us different **integrators**, each with its own tradeoffs between accuracy, stability, and cost.

---

### Explicit Euler: The Naive Approach

The simplest possible integrator assumes everything stays constant during the timestep:

```
v(t + dt) = v(t) + a(t) * dt
x(t + dt) = x(t) + v(t) * dt
```

This is **Explicit (Forward) Euler**. It uses the *current* velocity to update position and the *current* acceleration to update velocity. It's easy to understand and implement, but it has a fundamental problem: **it's unstable**.

Euler integration is first-order — the error grows proportionally to `dt`. Under stiff forces (springs, constraints), it systematically *adds* energy to the system. A ball bouncing on a spring will bounce higher each time. An orbit simulation will spiral outward. The smaller `dt` is, the slower the divergence, but it never goes away.

### Semi-Implicit (Symplectic) Euler: A Critical Fix

A small but powerful rearrangement changes everything:

```
v(t + dt) = v(t) + a(t) * dt       ← update velocity FIRST
x(t + dt) = x(t) + v(t + dt) * dt  ← use the NEW velocity for position
```

The only difference from explicit Euler is the order: we update velocity first, then use *that updated velocity* to advance position. This is the **Semi-Implicit Euler** (also called Symplectic Euler), and despite being trivially different in code, its behavior is fundamentally better.

Semi-implicit Euler is a **symplectic integrator** — it preserves the geometric structure of the phase space (the position-velocity relationship). In practice, this means it conserves energy *on average*. A spring oscillation will wobble slightly around the true amplitude but never blow up. An orbit will stay in a stable path indefinitely.

Most real-time physics engines (Box2D, Bullet, PhysX) use some variant of symplectic integration for exactly this reason: it's cheap, it's stable, and it doesn't require complex state management.

### Velocity Verlet: Second-Order Accuracy

UMBRA goes one step further with **Velocity Verlet**, a second-order symplectic integrator. Where semi-implicit Euler uses only the *current* acceleration, Verlet uses the *average* of the old and new accelerations:

**Velocity update:**
```
v(t+dt) = v(t) + 0.5 * (a(t) + a(t+dt)) * dt
```

**Position update:**
```
x(t+dt) = x(t) + v(t) * dt + 0.5 * a(t) * dt²
```

The extra `0.5 * a * dt²` term in the position update is a second-order correction. It's small per frame, but it matters when bodies accelerate quickly — it prevents the "spiraling" artifacts you'd see with first-order methods on orbital or spring systems. The velocity averaging produces smoother trajectories and more accurate energy conservation.

The tradeoff: we need to store the previous frame's acceleration on each body so we can average it with the new one. A small memory cost for measurably better simulation quality.

---

### Newton's Laws: Where Forces Enter

So far we've discussed *how* to step position and velocity forward, but not *what drives them*. That's where Newton's laws come in.

**Newton's Second Law** states:

```
F = m * a    →    a = F / m
```

Force (`F`) causes acceleration (`a`), scaled by mass (`m`). A 10 N force on a 2 kg body produces `5 m/s²` of acceleration. The same force on a 20 kg body produces only `0.5 m/s²`. Mass is a body's resistance to linear acceleration — its **inertia**.

#### The Angular Equivalent: Torque and Rotational Inertia

Newton's second law has a direct rotational analog:

```
τ = I * α    →    α = τ / I
```

**Torque** (`τ`) is rotational force — it causes angular acceleration (`α`), scaled by the **moment of inertia** (`I`). Where mass resists linear acceleration, moment of inertia resists *rotational* acceleration. A long thin rod is harder to spin than a compact sphere of the same mass, because its mass is distributed farther from the pivot.

For 2D shapes with uniform density, the standard formulas are:

- **Circle (solid disk):** `I = ½mr²`
- **Rectangle (solid):** `I = (1/12) * m * (w² + h²)`

Getting inertia right matters — too low and boxes spin like tops, too high and they feel "frozen" rotationally. UMBRA auto-calculates inertia from the body's shape when the user doesn't provide a value.

---

### Putting It All Together: The Integration Pipeline

Each call to `PhysicsService::Step()` runs a strict pipeline. The first four stages handle integration:

```
1. IntegrateForces     →  forces become velocities
2. IntegrateVelocities →  velocities become positions
3. ApplyDamping        →  bleed off energy
4. ClearAccumulators   →  reset for next frame
5-11. Collision & solving (Parts 2-4)
```

Splitting integration into two phases is deliberate — it's required by the Velocity Verlet scheme.

#### Phase 1: Forces → Velocities (`IntegrateForces`)

This phase converts all accumulated forces into acceleration, then updates velocity using the Verlet average:

```cpp
// Newton's 2nd law: a = F * (1/m)
Vector2f newAcceleration = body.ForceAccumulated * body.InverseMass;
float newAngularAcceleration = body.TorqueAccumulated * body.InverseInertia;

// Velocity Verlet: average old + new acceleration
body.Velocity += (body.Acceleration + newAcceleration) * 0.5f * dt;
body.AngularVelocity += (body.AngularAcceleration + newAngularAcceleration) * 0.5f * dt;

// Store for next frame's averaging
body.Acceleration = newAcceleration;
body.AngularAcceleration = newAngularAcceleration;
```

Notice how the linear and angular updates are *structurally identical*. Force times inverse mass gives linear acceleration; torque times inverse inertia gives angular acceleration. The Verlet averaging applies to both.

#### Phase 2: Velocities → Positions (`IntegrateVelocities`)

The second phase applies the full Verlet displacement formula:

```cpp
// Verlet position: x = x + v*dt + 0.5*a*dt²
body.Position += body.Velocity * dt + body.Acceleration * (dt*dt * 0.5f);
body.Angle += body.AngularVelocity * dt + body.AngularAcceleration * (dt*dt * 0.5f);
```

Angles are normalized to `[0, 2π)` after integration to prevent floating-point drift over many rotations.

---

### Inverse Mass: The Static Body Trick

Every mass and inertia value is stored inverted:

```cpp
float InverseMass;     // 0 = infinite mass (static)
float InverseInertia;  // 0 = infinite inertia
```

This is a standard physics engine technique. Newton's law becomes `a = F * inverseMass` — a single multiplication instead of a division. But the real elegance is in how it handles static bodies: a body with `InverseMass = 0` naturally has zero acceleration from any force (`a = F * 0 = 0`), making it immovable without any special-casing. Static bodies, kinematic bodies, and sleeping bodies all flow through the same code paths — the inverse-mass simply zeros out their contributions.

### Gravity as a Force

Gravity isn't hardcoded as a special acceleration — it's applied as a *force* proportional to mass, fed through the same accumulator pipeline as every other force:

```cpp
if (body.bAffectedByGravity && body.InverseMass > 0) {
    Vector2f gravityForce = Gravity / body.InverseMass;  // F = m * g
    body.ForceAccumulated += gravityForce;
}
```

Since the engine stores `InverseMass` (1/m), dividing gravity by inverse mass gives `m * g`. When this force reaches `IntegrateForces`, the mass cancels out: `a = (m * g) * (1/m) = g`. All objects accelerate equally under gravity regardless of mass — as Galileo demonstrated — while gravity remains in the force pipeline, making features like per-body gravity flags trivial.

### Exponential Damping

Real-world objects experience drag. A naive implementation (`v *= 0.99`) is frame-rate dependent — at 100fps you'd apply it 100 times per second, at 50fps only 50. UMBRA uses **exponential damping**:

```cpp
body.Velocity *= pow(damping, dt);
body.AngularVelocity *= pow(angularDamping, dt);
```

With `damping = 0.975`, at 100fps (dt=0.01) you get `0.975^0.01 ≈ 0.99975` per frame, and at 50fps (dt=0.02) you get `0.975^0.02 ≈ 0.99949` — the rate of energy loss over *wall-clock time* is identical regardless of frame rate. Each body can override the global damping values for per-body tuning.

### The Force Pipeline

Forces and impulses are the two ways to affect a body's motion, and the distinction matters:

```
ApplyForce(F)           → adds F to accumulator (integrated over dt)
ApplyForceAtPoint(F, p) → adds F + generates torque from lever arm
ApplyTorque(τ)          → adds τ to torque accumulator
ApplyImpulse(J)         → directly changes velocity: Δv = J * inverseMass
ApplyImpulseAtPoint(J, p) → velocity + angular change from cross product
```

**Forces** are accumulated throughout the frame, integrated during `IntegrateForces`, then cleared. They represent continuous pushes — gravity, springs, thrusters. **Impulses** bypass the accumulator entirely and modify velocity directly. They represent instantaneous events — collision response, explosions. The collision solver in Part 3 uses impulses; gameplay systems typically use forces.

`ApplyForceAtPoint` deserves special attention. When a force is applied off-center, it produces both a linear force and a **torque** proportional to the lever arm:

```
τ = r × F    (2D cross product)
```

where `r` is the vector from the body's center of mass to the application point. A force through the center produces pure translation; a force at the edge also spins the body. This is how the collision solver generates realistic rotational responses from contact impulses.

---

## Part 2 — Collision Detection: From Overlap Tests to Contact Points

Integration tells us where bodies *want* to go. Collision detection tells us where they *can't*. This part covers the progression from simple shape tests to the full contact generation pipeline that feeds into the solver.

---

### What a Collision Test Must Produce

Before diving into specific algorithms, it helps to understand what the solver in Part 3 actually needs. Every collision test in UMBRA fills a `CollisionDef` structure:

```cpp
struct CollisionDef {
    Vector2f contactNormal;     // unit vector pointing from body A toward body B
    float penetration;          // overlap depth along that normal
    Vector<ContactDef> contacts; // one or more contact points
};

struct ContactDef {
    Vector2f contactPoint;      // world-space location of contact
    float penetration;          // per-point penetration depth
};
```

The **contact normal** tells the solver which direction to push the bodies apart. The **penetration depth** tells it how far. And the **contact points** tell it *where* to apply the impulses — which determines how the collision affects rotation.

UMBRA enforces an **A→B normal convention** everywhere. The normal always points from body A toward body B. When an algorithm naturally produces a B→A normal, the dispatcher flips it. This consistency is critical — the solver relies on it to apply impulses in the correct directions.

---

### Circle vs Circle: The Simplest Collision

Two circles are the easiest shapes to test for overlap. The geometry is trivial: two circles intersect if and only if the distance between their centers is less than the sum of their radii.

The naive approach would compute:

```
distance = |posB - posA|
if distance < radiusA + radiusB → collision
```

But computing `|posB - posA|` requires a square root, which is expensive. Since we only need to *compare* distances for the early-out, we can work in squared space:

```cpp
Vector2f displacement = posB - posA;
float distSq = displacement.SquareMagnitude();
float radiiSum = radiusA + radiusB;

if (distSq > radiiSum * radiiSum) return false;  // no overlap
```

If the squared distance exceeds the squared radii sum, the circles are separated — no square root needed. We only compute the actual distance when we know there's a collision and need the penetration depth.

Once overlap is confirmed:

```cpp
float dist = sqrt(distSq);
contactNormal = displacement / dist;           // unit vector from A toward B
penetration = radiiSum - dist;                 // how far they overlap
contactPoint = posA + contactNormal * radiusA; // point on A's surface toward B
```

The **contact normal** is simply the normalized vector between centers — for circles, the deepest penetration is always along the center-to-center line. The **contact point** sits on circle A's surface, at the spot closest to B. And the **penetration** is how much the two surfaces overlap along that line.

There's one degenerate case: if the two circles have the exact same center position, the displacement vector is zero and can't be normalized. UMBRA handles this by falling back to an arbitrary axis `(1, 0)`. The choice doesn't matter physically — any direction will push them apart.

---

### Axis-Aligned Bounding Boxes (AABBs): The Broadphase Primitive

Before explaining the more complex tests, it's worth understanding **AABBs** (Axis-Aligned Bounding Boxes), since they underpin the broadphase system covered in Part 4 and appear throughout the engine.

An AABB is defined by a center position and a size (or equivalently, a min corner and a max corner). Its edges are always parallel to the coordinate axes — no rotation. This restriction makes overlap testing extremely cheap:

```
Two AABBs overlap if and only if they overlap on BOTH axes:
    overlapX = (A.min.x <= B.max.x) AND (A.max.x >= B.min.x)
    overlapY = (A.min.y <= B.max.y) AND (A.max.y >= B.min.y)
    collision = overlapX AND overlapY
```

This is just four comparisons — no dot products, no square roots, no trigonometry. The tradeoff is that AABBs can't represent rotated shapes tightly; a rotated rectangle's AABB wastes space at the corners. But for *quickly ruling out* pairs that are nowhere near each other, AABBs are unbeatable. The broadphase uses them to reduce thousands of potential pairs to a handful of actual candidates.

For narrow-phase collision testing between actual game objects, UMBRA uses **OBBs** (Oriented Bounding Boxes) — boxes that *can* rotate — tested with the Separating Axis Theorem.

---

### Circle vs OBB (Oriented Bounding Box)

Testing a circle against a rotated box is more involved than circle-circle, but the core idea is elegant: find the **closest point on the box's boundary** to the circle's center, then check if that point is within the circle's radius.

The challenge is that the box is rotated. Computing the closest point on an axis-aligned box is trivial (just clamp each coordinate to the box's extent), but an OBB's edges aren't aligned with anything convenient. The solution: **transform into the box's local space**.

#### The Closest-Point-on-OBB Algorithm

`GetClosestPointOnOrientedBoundEdge` works in three steps:

**Step 1: Project into OBB local space.** The box has two basis vectors — its rotated X and Y axes. Project the vector from the box's center to the circle's center onto these axes:

```cpp
Vector2f centerToPoint = circlePos - bounds.Center;
Vector2f rotatedU = Vector2f(1, 0).GetRotated(angle);  // box's local X axis
Vector2f rotatedV = Vector2f(0, 1).GetRotated(angle);  // box's local Y axis

float projOnU = Dot(rotatedU, centerToPoint);  // how far along X
float projOnV = Dot(rotatedV, centerToPoint);  // how far along Y
```

In the box's local coordinate frame, the problem becomes axis-aligned — we just need to clamp to the half-extents.

**Step 2: Clamp to the box boundary.** If the point is outside the box, clamp each projection to `[-halfSize, +halfSize]`. If the point is *inside* the box (the circle center has penetrated the box), find the nearest edge by comparing distances to each side:

```cpp
if (insideBox) {
    float distToEdgeU = halfSize.x - abs(projOnU);
    float distToEdgeV = halfSize.y - abs(projOnV);
    // Push to the closer edge
    if (distToEdgeV > distToEdgeU)
        projOnU = sign(projOnU) * halfSize.x;
    else
        projOnV = sign(projOnV) * halfSize.y;
} else {
    projOnU = Clamp(projOnU, -halfSize.x, halfSize.x);
    projOnV = Clamp(projOnV, -halfSize.y, halfSize.y);
}
```

**Step 3: Transform back to world space.** Reconstruct the world-space point from the (possibly clamped) local projections:

```cpp
closestPoint = bounds.Center + rotatedU * projOnU + rotatedV * projOnV;
```

#### Building the Collision Result

With the closest point on the OBB in hand, the rest is straightforward:

```cpp
Vector2f pointOnOBB = GetClosestPointOnOrientedBoundEdge(bounds, angle, circlePos);
float distance = (circlePos - pointOnOBB).Magnitude();
if (distance > radius) return false;

penetration = radius - distance;
contactNormal = (circlePos - pointOnOBB) / distance;
contactPoint = pointOnOBB;
```

The contact point *is* the closest point on the OBB surface. The normal points from that surface point toward the circle center. This naturally handles all geometric cases — the circle touching a face, grazing a corner, or sitting against an edge — because the closest-point algorithm handles them all uniformly.

Note the normal direction: this algorithm produces a normal pointing from the **box toward the circle**. When the dispatcher receives a Circle-vs-Box pair (where the circle is body A), it flips the normal to maintain the A→B convention.

---

### The Separating Axis Theorem (SAT): Box vs Box

Circle tests exploit the symmetry of circles. Box-vs-box collision doesn't have that luxury — two rotated rectangles can overlap in complex ways. The algorithm that handles this is the **Separating Axis Theorem**, the workhorse of convex polygon collision detection.

#### The Theorem

SAT is based on a powerful geometric observation:

> **Two convex shapes do NOT overlap if and only if there exists a line (axis) such that the projections of the two shapes onto that line do not overlap.**

Think of it like casting shadows. Imagine shining a light perpendicular to some axis and looking at the shadows both shapes cast. If you can find *any* angle where the shadows don't touch, the shapes are separated. If the shadows overlap from *every* angle, the shapes must be colliding.

The critical insight that makes SAT practical: **you don't need to test every possible axis.** For two convex polygons, the only axes you need to test are the **edge normals** of both shapes. These are the only directions where a separating gap could first appear as you rotate through possible axes. For two boxes (4 edges each), that's 8 candidate axes — though parallel edges reduce this to at most 4 unique directions.

#### Why Edge Normals?

Consider a box resting on a floor (another box). The gap between them vanishes along the floor's surface normal — that's where contact happens. If you projected along any other direction, you'd see overlap that doesn't represent the actual contact geometry. The edge normals are the directions that are *perpendicular* to the faces of each shape, making them the natural candidates for detecting face-to-face separation.

#### Step 1: Build World-Space Polygons

Each box is stored as a center position, a size, and a rotation angle. To perform SAT, we first convert each box into 4 world-space vertices:

```cpp
vertices.emplace_back(pos + Vector2f( halfW, -halfH).GetRotated(angle));
vertices.emplace_back(pos + Vector2f( halfW,  halfH).GetRotated(angle));
vertices.emplace_back(pos + Vector2f(-halfW,  halfH).GetRotated(angle));
vertices.emplace_back(pos + Vector2f(-halfW, -halfH).GetRotated(angle));
```

Each corner is offset from center by the half-extents, then rotated by the body's angle. The resulting `Polygon` object can compute its edge normals and project itself onto arbitrary axes.

#### Step 2: Project and Test All Axes

For each candidate axis (the edge normals from both polygons), we project all vertices of both shapes onto that axis and check whether the resulting intervals overlap:

```cpp
for (Vector2f axis : allAxes) {
    Projection projA = shapeA.GetProjectionOntoAxis(axis);  // {min, max}
    Projection projB = shapeB.GetProjectionOntoAxis(axis);

    // If projections don't overlap → separating axis found → no collision
    if (projA.Min > projB.Max || projA.Max < projB.Min) {
        return false;
    }

    // Track the axis with the smallest overlap
    float overlap = min(projA.Max, projB.Max) - max(projA.Min, projB.Min);
    if (overlap < minOverlap) {
        minOverlap = overlap;
        minAxis = axis;
    }
}
```

Projecting onto an axis means taking the dot product of each vertex with the axis direction and recording the minimum and maximum scalar values. Two intervals `[minA, maxA]` and `[minB, maxB]` overlap if and only if `minA <= maxB AND maxA >= minB`.

If *any* axis produces non-overlapping intervals, the shapes are separated and we return early. If *all* axes overlap, the shapes are colliding.

#### The Minimum Translation Vector (MTV)

When all axes overlap, we don't just know the shapes collide — we also know *how to separate them*. The axis with the **smallest overlap** is the **Minimum Translation Vector** direction, and that smallest overlap is the penetration depth. This is the cheapest direction to push the shapes apart.

#### Step 3: Orient the Normal

The MTV axis has an arbitrary sign — it might point from A→B or B→A. We fix this by checking it against the center-to-center vector:

```cpp
Vector2f AB = shapeB.GetCenter() - shapeA.GetCenter();
if (Dot(AB, minAxis) < 0) {
    minAxis = -minAxis;
}
```

Now `contactNormal` reliably points from A toward B, matching the engine's convention.

---

### Contact Point Generation: Where Do the Shapes Actually Touch?

SAT gives us the collision normal and penetration depth, but the solver needs more — it needs to know the specific **contact points** where the shapes touch. Why? Because the *location* of a contact determines how the impulse affects rotation.

Imagine a box sitting on a flat floor. If we model this as a single contact point at the center, the box could freely rotate around that point — it would wobble endlessly. With *two* contact points at the box's bottom corners, the solver can resist rotation because any spinning motion would push one contact deeper while pulling the other apart. Two contacts create a **moment arm** that stabilizes stacking.

UMBRA uses **Sutherland-Hodgman polygon clipping** to find these contact points. The idea: identify the edges of both shapes that are closest to the collision interface, then clip one against the other to find the overlapping region.

#### Step 1: Find the Support Vertex and Best Edge

For each polygon, we find the **support vertex** — the vertex that projects farthest along the contact normal (for shape A) or against the normal (for shape B). This is the vertex most "in front" with respect to the collision direction.

The **best edge** is the edge adjacent to the support vertex that is *most perpendicular* to the contact normal — the edge most aligned with the collision surface:

```cpp
// Two edges meet at the support vertex. Pick the more perpendicular one.
float prevProj = Dot(prevEdgeDir, normal);
float nextProj = Dot(nextEdgeDir, normal);
if (abs(prevProj) <= abs(nextProj))
    bestEdge = {prevVertex, supportVertex};
else
    bestEdge = {supportVertex, nextVertex};
```

The edge with the *smaller* absolute dot product against the normal is more perpendicular — it's more "face-like" than "edge-like" relative to the contact direction.

#### Step 2: Reference Edge vs Incident Edge

We now have a best edge from each shape. The edge that is *more perpendicular* to the contact normal becomes the **reference edge** — it defines the clipping boundary, the "wall" we clip against. The other edge is the **incident edge** — it gets clipped.

```cpp
float e1Dot = abs(Dot(bestEdgeA_dir, normal));
float e2Dot = abs(Dot(bestEdgeB_dir, normal));
if (e1Dot <= e2Dot) {
    referenceEdge = bestEdgeA;   // more perpendicular → it's the "wall"
    incidentEdge = bestEdgeB;    // gets clipped against the wall
} else {
    referenceEdge = bestEdgeB;
    incidentEdge = bestEdgeA;
}
```

Think of it geometrically: the reference edge is the flat surface that the other edge is pressing into. When a small box sits on a large floor, the floor's top edge is the reference and the box's bottom edge is the incident.

#### Step 3: Sutherland-Hodgman Clipping

The incident edge is now clipped against the two **side planes** of the reference edge. These side planes are perpendicular to the reference edge at each endpoint — they define the "extent" of the reference face.

```cpp
// Clip against the left side plane
Clip(refEdgeDir, incidentEdge, Dot(refEdgeDir, refEdge.first));
// Clip against the right side plane
Clip(-refEdgeDir, incidentEdge, Dot(-refEdgeDir, refEdge.second));
```

The `Clip` function implements Sutherland-Hodgman for a single half-plane. For each endpoint of the incident edge, it computes the signed distance to the clipping plane:

- Points on the positive side (inside) are kept
- Points on the negative side (outside) are discarded
- When the edge crosses the plane, an intersection point is computed via linear interpolation

```cpp
float dist1 = Dot(normal, edge.first) - clippingPlaneProj;
float dist2 = Dot(normal, edge.second) - clippingPlaneProj;

if (dist1 >= 0) keep(edge.first);
if (dist2 >= 0) keep(edge.second);

if (dist1 * dist2 < 0) {  // opposite sides → edge crosses the plane
    float t = dist1 / (dist1 - dist2);
    newPoint = edge.first + t * (edge.second - edge.first);
    keep(newPoint);
}
```

The `t` parameter gives the fractional position along the edge where it crosses the clipping plane. `dist1 / (dist1 - dist2)` is a standard linear interpolation formula — when `dist1` and `dist2` have opposite signs, `t` falls between 0 and 1.

After clipping against both side planes, the incident edge has been trimmed to only the portion that lies within the reference face's extent.

#### Step 4: Depth Filter

The clipped points aren't all valid contacts — some may lie in front of the reference face rather than behind it. We keep only points that are *behind* (or on) the reference face, meaning they represent actual penetration:

```cpp
float refDepth = Dot(refNormal, refEdge.first);  // reference face plane
for (auto& point : clippedPoints) {
    float depth = Dot(refNormal, point) - refDepth;
    if (depth <= 0.0f) {
        contact.contactPoint = point;
        contact.penetration = -depth;  // flip sign so penetration is positive
        contacts.push_back(contact);
    }
}
```

The result: **1 contact point** for vertex-face collisions (a corner poking into a flat surface) and **2 contact points** for face-face collisions (two edges pressing against each other). Those two points are what make stable stacking possible — they give the solver the leverage it needs to resist unwanted rotation.

---

### The Dispatch Table

With all three collision types implemented, `CollisionQuery::CheckCollision` routes each body pair to the appropriate algorithm:

```
Circle vs Circle  →  TestCircleCircle
Circle vs Box     →  TestCircleVsOBB  (normal flipped to maintain A→B)
Box    vs Circle  →  TestCircleVsOBB  (argument order swapped)
Box    vs Box     →  TestBoxVsBoxSAT  →  TestPolygonVsPolygonOverlapSAT
                                       →  GetContactPointsViaClipping
```

The Circle-vs-Box case appears twice because the argument order matters. When the circle is body A and the box is body B, we call `TestCircleVsOBB` with the circle first, but the resulting normal points from the box surface toward the circle — the opposite of our A→B convention. So the dispatcher flips it. When the box is body A and the circle is body B, we swap the arguments and the natural normal direction already matches.

---

## Part 3 — Contact Resolution: How Bodies React to Collisions

Parts 1 and 2 established two halves of the simulation: integration moves bodies forward, and collision detection finds where they overlap. But detection alone doesn't *do* anything — it just reports geometry. The solver's job is to take that geometric information and compute the physical response: how fast should each body bounce, slide, or stop? This is where the math gets interesting.

---

### The Problem: What Should Happen at a Contact?

When two bodies overlap, we need to:

1. **Prevent penetration** — push them apart so they don't pass through each other
2. **Simulate bounce** — fast collisions should rebound based on material properties
3. **Simulate friction** — surfaces should resist sliding against each other
4. **Handle multiple contacts simultaneously** — a box on the floor has two contact points, and what we do at one affects the other

The naive approach — computing each collision response independently in a single pass — fails badly. Contacts share bodies. Correcting one contact changes the velocities that another contact depends on. A box sitting on a floor with two contact points would jitter endlessly as each contact's correction undoes the other's.

### The Sequential Impulse Method

UMBRA solves this with **Sequential Impulse** (SI), the same algorithm that powers Box2D and most real-time physics engines. The core idea is simple:

1. **Precompute** everything that stays constant during solving (effective masses, bias terms) — do this once per frame
2. **Iterate** over all contacts multiple times, applying small corrective impulses at each one
3. Each iteration partially corrects the error left by the previous pass — with enough iterations, the system **converges** to a globally consistent solution

This is mathematically equivalent to **Gauss-Seidel iteration** applied to a system of inequality constraints. Each pass solves one contact assuming all others are fixed, then moves to the next. Because each solve uses the *latest* velocities (updated by previous contacts in the same pass), information propagates through chains of contacts within a single iteration. A stack of 5 boxes needs only ~5 iterations for a correction at the bottom to reach the top.

---

### Precomputation: Setting Up the Constraints

Before iterating, we compute everything that doesn't change between solver iterations. This work happens once per frame in `PrecomputeContactConstraints()`.

#### Lever Arms

```cpp
contact.rA = contactPoint - bodyA.Position;
contact.rB = contactPoint - bodyB.Position;
```

These vectors point from each body's center of mass to the contact point. They determine how an impulse at the contact creates **torque** — a force applied far from the center spins the body more than one applied near it. The lever arm is the rotational equivalent of mass: it controls how much of the impulse goes into rotation versus translation.

#### Effective Mass: Combining Translation and Rotation

An impulse at a contact point doesn't just push bodies apart linearly — it also spins them. The **effective mass** captures the total "resistance" that both bodies present to an impulse along a given direction, accounting for both effects.

For an impulse along the contact normal `n`:

```
1/m_eff = 1/mA + 1/mB + (rA × n)²/IA + (rB × n)²/IB
```

The first two terms are the familiar inverse masses — how easily each body translates. The last two terms are the *rotational contribution*: `rA × n` is the 2D cross product of the lever arm with the impulse direction, and squaring it and dividing by inertia gives the rotational "give" at the contact point.

In code:

```cpp
float rACrossN = Cross2D(rA, normal);
float rBCrossN = Cross2D(rB, normal);
float normalDenom = invMassSum
    + rACrossN * rACrossN * invInertiaA
    + rBCrossN * rBCrossN * invInertiaB;
contact.normalMass = 1.0f / normalDenom;
```

**Why does this matter?** A contact at the center of mass has `rA × n = 0`, so none of the impulse goes into rotation — the body's full mass resists the push. A contact at the edge of a large box has a big lever arm, so part of the impulse goes into spinning the box, making the contact effectively "softer." The effective mass captures this automatically.

#### Effective Mass (Tangent Direction)

The exact same formula applies for the friction direction (the tangent, perpendicular to the normal):

```cpp
Vector2f tangent(-normal.y, normal.x);
float rACrossT = Cross2D(rA, tangent);
float rBCrossT = Cross2D(rB, tangent);
float tangentDenom = invMassSum
    + rACrossT * rACrossT * invInertiaA
    + rBCrossT * rBCrossT * invInertiaB;
contact.tangentMass = 1.0f / tangentDenom;
```

This tells the friction solver how the contact resists sliding motion, again accounting for both translation and rotation.

#### Restitution Velocity Bias

**Restitution** (bounciness) determines how much kinetic energy is preserved in a collision. A coefficient of restitution `e = 1` means a perfectly elastic bounce (a tennis ball on concrete); `e = 0` means the impact is fully absorbed (a lump of clay).

During precomputation, we measure the **closing speed** — how fast the bodies are approaching along the contact normal — and use it to compute a velocity bias:

```cpp
// Velocity at each contact point (includes rotational contribution)
Vector2f velA = bodyA.Velocity + Vector2f(-rA.y, rA.x) * bodyA.AngularVelocity;
Vector2f velB = bodyB.Velocity + Vector2f(-rB.y, rB.x) * bodyB.AngularVelocity;
float closingSpeed = Dot(velB - velA, normal);

// Only apply restitution above a speed threshold
contact.velocityBias = closingSpeed < -1.0f ? -e * closingSpeed : 0.0f;
```

The velocity at the contact point isn't just the body's linear velocity — a spinning body has additional velocity at its surface. `Vector2f(-rA.y, rA.x) * angularVelocity` computes the tangential velocity from rotation at the contact point.

The `-1.0f` threshold is a critical stability feature. Without it, bodies resting on surfaces would micro-bounce forever — tiny numerical velocities would trigger restitution, adding energy each frame. By requiring a minimum closing speed before applying bounce, resting contacts absorb tiny motions completely.

---

### The Velocity Solver: Iterating Toward Convergence

With precomputation done, the solver runs `VelocityIterations` times (default: 8). Each iteration loops over every contact and applies two impulses: one along the normal (separation) and one along the tangent (friction).

#### Normal Impulse: Preventing Penetration

The goal: after the impulse, the relative velocity along the contact normal should be zero (or positive, if restitution adds bounce). The required impulse magnitude is:

```
impulse = (-velAlongNormal + velocityBias) * effectiveMass
```

This is Newton's second law in impulse form: the change in velocity we want, divided by how easily the system can change (the effective mass).

```cpp
// Current relative velocity at contact point
Vector2f relVel = velB - velA;
float velAlongNormal = Dot(relVel, normal);

// How much impulse we need this iteration
float dj = (-velAlongNormal + velocityBias) * normalMass;
```

But we can't just apply `dj` directly. There's a critical constraint: **the contact normal impulse must never be negative.** A contact can push bodies apart but not pull them together — if it could, objects would stick to surfaces like glue.

#### Accumulated Clamping: The Key to Stability

The naive approach would be to clamp each iteration's impulse: `dj = max(dj, 0)`. This works for isolated contacts but fails for systems with multiple contacts on the same body. Consider a box with two contacts on the floor. Correcting the left contact might over-push the right side down, and with per-iteration clamping, we can never *undo* that excess — we can only add more impulse, never subtract.

UMBRA uses **accumulated clamping** instead, the same technique Box2D pioneered:

```cpp
float oldAccum = contact.normalImpulseAccum;
contact.normalImpulseAccum = max(oldAccum + dj, 0.0f);  // clamp the TOTAL
dj = contact.normalImpulseAccum - oldAccum;  // compute the actual delta
```

The constraint is applied to the **total accumulated impulse**, not the per-iteration delta. This means `dj` can be *negative* — pulling back impulse that was over-applied in a previous iteration. The total is always non-negative (contacts only push apart), but individual iterations can course-correct. This converges to the correct solution dramatically faster.

#### Applying the Impulse

Once we have the corrected `dj`, it's applied to both bodies — equal and opposite, as Newton's third law demands:

```cpp
bodyA.Velocity -= normal * dj * invMassA;
bodyA.AngularVelocity -= Cross2D(rA, normal * dj) * invInertiaA;
bodyB.Velocity += normal * dj * invMassB;
bodyB.AngularVelocity += Cross2D(rB, normal * dj) * invInertiaB;
```

The linear velocity changes by `impulse * inverseMass` (Newton's second law for impulses). The angular velocity changes by `Cross2D(r, impulse) * inverseInertia` — the cross product of the lever arm with the impulse gives the torque, scaled by inverse inertia. A centered hit produces no spin; an off-center hit produces both translation and rotation.

#### Friction Impulse: Resisting Sliding

After the normal impulse has updated velocities, the solver handles **friction** — the force that resists sliding motion along the contact surface.

The friction impulse is computed along the **tangent** direction (perpendicular to the contact normal):

```cpp
float velAlongTangent = Dot(relVel, tangent);
float djt = -velAlongTangent * tangentMass;
```

If this were unclamped, friction would perfectly zero out all sliding — everything would be perfectly sticky. Real friction has a limit governed by **Coulomb's friction law**:

```
|friction impulse| ≤ μ * normal impulse
```

The friction force can never exceed `μ` (the friction coefficient) times the normal force holding the surfaces together. Below this limit, surfaces **stick** (static friction). At the limit, surfaces **slide** (dynamic friction).

```cpp
float maxFriction = friction * contact.normalImpulseAccum;
float oldTangentAccum = contact.tangentImpulseAccum;
contact.tangentImpulseAccum = clamp(oldTangentAccum + djt, -maxFriction, maxFriction);
djt = contact.tangentImpulseAccum - oldTangentAccum;
```

The same accumulated clamping technique applies — the *total* tangent impulse is clamped, not the per-iteration delta. The clamp is symmetric (`-maxFriction` to `+maxFriction`) because friction resists sliding in either tangential direction.

The friction coefficient between two bodies is computed as the **geometric mean** of their individual friction values:

```cpp
float friction = sqrt(bodyA.DynamicFriction * bodyB.DynamicFriction);
```

The geometric mean has a useful property: if either surface has zero friction, the pair is frictionless (`sqrt(0 * x) = 0`). Two high-friction surfaces produce a combined friction higher than either alone, which matches physical intuition.

---

### Position Correction: Fixing What Velocity Can't

The velocity solver prevents *future* penetration — it ensures bodies are moving apart after the impulse. But bodies may already be overlapping *right now*, and velocity changes alone don't fix existing overlap. Left uncorrected, you'd see bodies embedded in floors and walls.

**Position correction** directly nudges overlapping bodies apart:

```cpp
const float percent = 0.4f;  // correct 40% of penetration per iteration
const float slop = 0.01f;    // allow 1cm of overlap before correcting

float correctionMag = max(penetration - slop, 0.0f);
Vector2f correction = normal * (correctionMag / invMassSum) * percent;

bodyA.Position -= correction * invMassA;
bodyB.Position += correction * invMassB;
```

Two parameters control stability:

**Slop** (penetration allowance). Without slop, the solver fights numerical drift every frame — bodies at rest would jitter as the solver repeatedly tries to achieve exactly zero penetration. The 1cm slop means contacts can slightly overlap, which is invisible at game scale but eliminates jitter completely.

**Correction percentage** (0.4). Correcting 100% of penetration in one shot causes oscillation — the bodies overshoot, overlap from the other side, and bounce back and forth. Correcting only 40% per iteration and running 3 position iterations (the default `PositionIterations`) converges smoothly. This is **Baumgarte stabilization** — the standard technique for position-level constraint correction.

The correction is distributed between bodies proportional to their inverse masses. A heavy body barely moves; a light body moves a lot. A static body (`InverseMass = 0`) doesn't move at all — the entire correction applies to the dynamic body, which is exactly the right behavior for a ball resting on an immovable floor.

---

### Constraint Joints: Springs, Distances, and Hinges

Contact constraints aren't the only constraints in a physics engine. **Joints** connect bodies together — springs pull them toward a rest length, distance constraints keep them at a fixed separation, and hinges pin them at a shared point while allowing rotation.

UMBRA solves joints alongside contacts in the same iteration loop. Each constraint type has its own precomputation and solve step, but they all share the same mathematical framework: compute an effective mass, measure the constraint error, and apply a corrective impulse.

#### Spring Constraints (Force-Based)

Springs are the simplest joint. They apply a force proportional to the displacement from a rest length (**Hooke's law**) plus a damping term:

```
F = k * (currentLength - restLength) + c * relativeVelocity
```

where `k` is stiffness and `c` is damping. Unlike contacts, springs are solved as **forces** rather than impulses — they're applied before integration, in the force accumulation phase:

```cpp
float forceMag = spring.Stiffness * (currentLength - spring.RestLength)
               + spring.Damping * relVel;
Vector2f force = direction * forceMag;

bodyA.ForceAccumulated += force;
bodyA.TorqueAccumulated += Cross2D(rA, force);
bodyB.ForceAccumulated -= force;
bodyB.TorqueAccumulated -= Cross2D(rB, force);
```

The force is applied at the anchor points (which may be offset from the center of mass), so it generates both a linear force and torque via the cross product — a spring attached to the corner of a box will both pull *and* spin it.

#### Distance Constraints (Impulse-Based)

A distance constraint maintains a fixed separation between two anchor points. Unlike a spring, it doesn't oscillate — it rigidly enforces the distance.

The precomputation follows the same pattern as contacts: compute lever arms, compute the effective mass along the constraint axis, and compute a **Baumgarte bias** to correct positional drift:

```cpp
float bias = baumgarteScale * inverseDt * (currentDistance - desiredDistance);
```

The velocity solver computes a corrective impulse:

```cpp
float relVelAlongAxis = Dot(relativeVelocity, axis);
float lambda = effectiveMass * -(relVelAlongAxis + bias);
```

This impulse is accumulated across iterations, and applied to both bodies along the constraint axis.

#### Hinge Constraints (2D Revolute Joint)

A hinge pins two bodies at a shared point — they can rotate relative to each other but can't separate. In 2D, this is a **2-DOF position constraint**: the anchor point on body A must always coincide with the anchor point on body B, constraining both X and Y.

Solving a 2-DOF constraint requires a **2x2 effective mass matrix** rather than a scalar:

```cpp
// K = [Kxx, Kxy]
//     [Kxy, Kyy]
float Kxx = invMassSum + rAy*rAy*invIA + rBy*rBy*invIB;
float Kxy = -rAx*rAy*invIA - rBx*rBy*invIB;
float Kyy = invMassSum + rAx*rAx*invIA + rBx*rBx*invIB;
```

The constraint error is a 2D vector (the positional separation between the anchor points), and the system is solved via **Cramer's rule** — an efficient closed-form solution for 2x2 linear systems.

Hinges can also have **angular limits** (the relative angle between bodies is clamped to a range) and **motors** (a target angular velocity is driven by a torque impulse, clamped to a maximum motor torque). The angular limit impulse is uni-directional — it pushes only when the limit is violated — while the motor impulse is clamped symmetrically.

---

### Sleeping Bodies in the Solver

Throughout both the velocity solver and position correction, sleeping bodies are treated as having zero inverse mass and zero inverse inertia:

```cpp
float invMassA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
```

This makes sleeping bodies behave exactly like static bodies in the constraint math — they participate in collisions (providing support forces to dynamic bodies resting on them) but don't move themselves. It's cheaper than a full wake-up and prevents cascading wake-ups from minor numerical contacts.

### Why Multiple Iterations?

A single pass through all contacts produces a locally valid solution — each contact is satisfied in isolation. But contacts share bodies: fixing contact 1 changes body A's velocity, which may violate contact 2 (which also involves body A). Each additional iteration reduces this cross-talk error.

The convergence is roughly exponential — 8 iterations reduces the error by roughly `2^8 = 256x`, which is enough for stable stacking of 5-10 objects with responsive collisions and no visible jitter. More iterations improve stability (taller stacks, stiffer constraints) at the cost of CPU time. It's a tunable quality knob exposed as `VelocityIterations` and `PositionIterations` in the physics config.

---

## Part 4 — Broadphase, Dynamic AABB Tree, CCD, and Sleeping

The systems described so far — integration, collision detection, and constraint solving — operate on pairs of bodies. But *which* pairs? With `n` bodies in the scene, testing every pair produces `n(n-1)/2` tests. For 200 bodies, that's 19,900 narrow-phase collision checks per frame. Most of these pairs are nowhere near each other — testing SAT and computing contact points for bodies on opposite sides of the world is pure waste.

The **broadphase** exists to answer one question cheaply: *which pairs of bodies might possibly be colliding?* It reduces thousands of potential pairs to a handful of actual candidates, turning an O(n²) problem into something manageable.

---

### Why Broadphase Matters

The narrow-phase algorithms from Part 2 are not cheap. SAT requires building world-space polygons, projecting onto multiple axes, and potentially running Sutherland-Hodgman clipping. Even the simple circle-circle test involves a square root. Performing these tests on every possible pair is the single biggest performance bottleneck in a naive physics engine.

A good broadphase uses a *much cheaper* overlap test (AABB intersection — just 4 comparisons) on *approximate* bounding volumes to quickly rule out pairs that can't possibly collide. Only the surviving pairs are passed to the expensive narrow-phase.

### Choosing a Spatial Data Structure

There are several classic approaches to broadphase:

**Uniform Grid**: Divide the world into cells. Each body occupies one or more cells; only bodies in the same cell can collide. Fast and simple, but struggles with varying body sizes (a huge body touches many cells) and large worlds (most cells are empty).

**Quadtree**: A recursive spatial partition that subdivides regions with many bodies. Handles varying density well, but the tree is expensive to rebuild from scratch — and for a dynamic simulation where bodies move every frame, rebuilding is frequent.

**Dynamic AABB Tree**: A binary tree where leaf nodes hold bounding boxes for individual bodies, and internal nodes hold the union of their children's bounding boxes. The tree is *incrementally maintained* — bodies are inserted, removed, and moved without rebuilding. This makes it ideal for real-time simulation where most bodies only move slightly each frame.

UMBRA uses a **Dynamic AABB Tree**, the same structure Box2D uses. It provides O(log n) insertion, removal, and query time with minimal per-frame maintenance cost.

---

### How the Dynamic AABB Tree Works

#### The Node Structure

Each node in the tree stores an AABB and some bookkeeping:

```cpp
struct AABBTreeNode {
    Bounds2D Aabb;          // the bounding box
    int32 Parent;           // parent node index
    int32 Left, Right;      // child indices (NullNode for leaves)
    int32 Height;           // tree height for AVL balancing
    uint32 BodyIndex;       // physics body index (leaves only)
    int32 NextFree;         // free-list pointer for pool allocation
};
```

**Leaf nodes** represent individual physics bodies. Their AABB is a *fattened* version of the body's actual bounding box (more on this below). **Internal nodes** represent spatial groupings — their AABB is the union (the smallest box containing both children) of their children's AABBs.

#### Fat AABBs: Amortizing Tree Updates

A body's tight AABB changes every frame as it moves and rotates. Naively updating the tree every frame would be expensive and negate its benefits. The solution is **fat AABBs** — each leaf stores an enlarged bounding box with built-in slack.

When a body is inserted, its AABB is fattened by a uniform margin plus a velocity-based prediction:

```cpp
Bounds2D FattenAABB(const Bounds2D& aabb, const Vector2f& displacement) const {
    Vector2f margin(fattenMargin, fattenMargin);  // uniform expansion (default 0.1)
    Vector2f newMin = aabb.Min() - margin;
    Vector2f newMax = aabb.Max() + margin;

    // Extend in direction of movement
    Vector2f d = displacement * displacementMultiplier;  // default 2x
    if (d.x < 0) newMin.x += d.x;
    else          newMax.x += d.x;
    if (d.y < 0) newMin.y += d.y;
    else          newMax.y += d.y;

    return Bounds2D(center, size);
}
```

The uniform margin gives slack in all directions. The displacement-based extension predicts where the body is heading — a body moving right gets extra slack on the right side. The 2x multiplier means the fat AABB extends two frames worth of motion in the movement direction.

During `UpdateBroadphaseProxies`, if a body's current tight AABB still fits inside its fat AABB, the tree is not touched at all:

```cpp
bool MoveProxy(int32 proxyId, const Bounds2D& newAabb, const Vector2f& displacement) {
    if (nodes[proxyId].Aabb.Contains(newAabb)) return false;  // still fits

    RemoveLeaf(proxyId);
    nodes[proxyId].Aabb = FattenAABB(newAabb, displacement);
    InsertLeaf(proxyId);
    return true;
}
```

A smoothly-moving body might not trigger a tree update for many frames — the fat AABB absorbs the motion. Only when a body accelerates, changes direction, or moves enough to escape its fat envelope does the tree actually need modification. This converts the cost from "rebuild every frame" to "rebuild occasionally," which is a massive performance win.

#### Insertion: The Surface Area Heuristic

When inserting a new leaf, we need to choose where in the tree to place it. The wrong choice produces a lopsided tree with large, overlapping internal nodes — leading to more false positives during queries. The right choice keeps internal AABBs tight and the tree balanced.

UMBRA uses the **Surface Area Heuristic (SAH)**, the same approach Box2D uses. The idea: the "cost" of a node is proportional to the surface area (perimeter, in 2D) of its AABB, because larger AABBs are more likely to produce false-positive overlaps during queries. We want to minimize the total surface area increase caused by the insertion.

The algorithm traverses the tree from the root, at each internal node computing:

```
cost(node) = SA(union(node.Aabb, newLeaf.Aabb)) + inheritedCost
```

The inherited cost accounts for the fact that inserting here also enlarges every ancestor. A lower bound is computed at each step — if the lower bound already exceeds the best cost found so far, that entire subtree is pruned. This makes the search O(log n) for balanced trees instead of O(n).

Once the best sibling is found, a new internal node is created as the parent of both the existing sibling and the new leaf. The sibling's old parent becomes the grandparent.

#### AVL Balancing: Keeping the Tree Shallow

After insertion or removal, the tree walks upward from the modified node to the root, recomputing AABB unions and heights at each level. When the height difference between a node's left and right subtrees exceeds 1, an **AVL rotation** is applied:

```cpp
int32 balance = nodes[rightId].Height - nodes[leftId].Height;

if (balance > 1) {
    // Right-heavy: rotate right child up
    // Choose the taller grandchild to remain higher,
    // minimizing the resulting maximum height
}
if (balance < -1) {
    // Left-heavy: symmetric rotation
}
```

AVL balancing guarantees the tree stays at O(log n) depth, which in turn guarantees O(log n) query performance. Without balancing, pathological insertion orders could produce degenerate linear chains, turning O(log n) queries into O(n).

#### Querying: Finding Overlapping Pairs

The core operation of broadphase detection is: given a body's fat AABB, find all other leaves whose AABBs overlap it. The tree query is stack-based (avoiding recursion overhead and heap allocation):

```cpp
template <typename Func>
void Query(const Bounds2D& aabb, Func&& callback) const {
    Stack<int32> stack;
    stack.push(root);

    while (!stack.empty()) {
        int32 nodeId = stack.top(); stack.pop();
        const auto& node = nodes[nodeId];

        if (node.Aabb.Intersects(aabb)) {
            if (node.IsLeaf())
                callback(nodeId);    // potential collision pair
            else {
                stack.push(node.Left);
                stack.push(node.Right);
            }
        }
        // No intersection → prune entire subtree
    }
}
```

The key is the pruning: if an internal node's AABB doesn't intersect the query, *none of its descendants can either*, because every descendant's AABB is contained within the parent's. This prunes entire branches of the tree in constant time. In practice, the query visits O(log n + k) nodes, where k is the number of actual overlaps.

#### Pool Allocation: Zero-Cost Node Management

The tree uses a **free-list pool allocator** for node management, avoiding heap allocation during gameplay:

```cpp
// Pre-allocate nodes as a contiguous array with a linked free list
int32 AllocateNode() {
    if (freeList == NullNode) { /* double capacity, rebuild free list */ }
    int32 nodeId = freeList;
    freeList = nodes[nodeId].NextFree;
    return nodeId;
}

void FreeNode(int32 nodeId) {
    nodes[nodeId].NextFree = freeList;
    freeList = nodeId;
}
```

Allocation is O(1) — just pop from the free list. Deallocation is O(1) — push to the free list. When the pool runs out, it doubles in size (amortized O(1)). During steady-state simulation, this means zero heap allocations per frame from the tree.

---

### Broadphase Detection: Putting It Together

`BroadphaseDetection()` orchestrates the full broadphase pass:

```cpp
void BroadphaseDetection() {
    for (each active body i) {
        const Bounds2D& fatAABB = tree.GetFatAABB(bodies[i].TreeProxyId);

        tree.Query(fatAABB, [&](int32 proxyId) {
            uint32 otherIndex = tree.GetBodyIndex(proxyId);

            // Dedup: only keep pairs where i < other
            if (otherIndex <= i) return;

            // Skip if both sleeping
            if (bodies[i].bIsSleeping && bodies[otherIndex].bIsSleeping) return;

            // Collision filtering: check layer/mask compatibility
            if (!ShouldCollide(bodies[i], bodies[otherIndex])) return;

            overlappingPairs.emplace_back(handleA, handleB);
        });
    }
}
```

Three filters reduce the pair list beyond spatial proximity:

1. **Deduplication**: The `i < other` check ensures each pair is only reported once (A-B, not both A-B and B-A)
2. **Sleeping pairs**: If both bodies are asleep, they've already been resolved — skip them
3. **Collision filtering**: A bitmask system controls which layers can collide with which

#### Collision Filtering

Each body has a `CollisionFilter` with two bitmasks:

- **CategoryBits**: what layer this body belongs to (e.g., `0x0001` for players, `0x0002` for enemies)
- **MaskBits**: which layers this body collides with (e.g., `0xFFFF` for "everything")

Two bodies collide only if both directions agree:

```cpp
bool ShouldCollide(const Body& a, const Body& b) {
    return (a.CategoryBits & b.MaskBits) != 0
        && (b.CategoryBits & a.MaskBits) != 0;
}
```

This allows one-way filtering: bullets might collide with enemies but not with other bullets, even if both are on the same layer.

---

### Continuous Collision Detection (CCD)

Discrete collision detection — checking for overlap at each timestep — has a fundamental limitation: a fast-moving body can **tunnel** through a thin wall between frames. At frame N the bullet is in front of the wall; at frame N+1 it's behind it. No overlap was ever detected.

**Continuous Collision Detection** solves this by sweeping the body along its trajectory and finding the **time of impact** (TOI) — the earliest moment during the frame when a collision would occur.

#### When CCD Activates

CCD is expensive, so UMBRA only applies it to bodies that are moving fast enough to potentially tunnel. The threshold is based on the body's size:

```cpp
float extent = (body is circle) ? 2 * radius : min(width, height);
float threshold = extent * CCDMotionThreshold;  // default 0.5

if (displacement.Magnitude() > threshold) {
    // perform CCD sweep
}
```

A body must move more than half its own size in a single frame to trigger CCD. Small, slow objects never pay the cost.

#### The CCD Pipeline

1. **Save positions** before integration: `body.CCDSavedPosition = body.Position`
2. After integration moves the body to its new position, **build a swept AABB** — the union of the old and new position AABBs
3. **Query the broadphase tree** for all bodies overlapping the swept region
4. For each candidate, compute the **time of impact**
5. **Clamp the position** to the earliest TOI: `position = savedPos + displacement * minTOI`

#### Time-of-Impact Algorithms

Each shape pair has its own TOI computation:

**Circle vs Circle**: Reduces to a quadratic equation. The swept circle center traces a line segment; we solve for the time `t` when the distance between the sweeping center and the stationary center equals the sum of radii:

```
|p0 + t*d - pB|² = (rA + rB)²
```

This expands to `at² + bt + c = 0`, solvable in closed form with the quadratic formula. The smallest positive root in `[0, 1]` is the TOI.

**Circle vs OBB**: The problem is transformed into the OBB's local space (making it axis-aligned), the box is expanded by the circle's radius (Minkowski sum), and a standard **ray-slab intersection** test finds the TOI. The slab method clips the ray against each axis's min/max planes, tracking the latest entry and earliest exit.

**Box vs Box**: No closed-form solution exists for sweeping two oriented boxes. UMBRA uses **binary bisection**: sample the trajectory at midpoints, test for overlap using SAT at each sample, and narrow down the collision window over multiple iterations (default: 8 bisection steps).

---

### Body Sleeping: Skipping What Doesn't Need Simulating

Once a stack of boxes settles, continuing to simulate it is wasted work. Every frame, the integrator computes tiny velocities, the broadphase checks for overlaps that haven't changed, the solver applies impulses that cancel out. The **sleeping system** detects bodies at rest and removes them from the active simulation entirely.

#### Sleep Criteria

A body becomes a sleep candidate when both its linear and angular velocities drop below configurable thresholds. But a brief pause isn't enough — a ball at the apex of its arc has momentarily zero velocity. The body must stay below the thresholds for a sustained duration:

```cpp
bool belowThreshold = body.Velocity.SquareMagnitude() < sleepLinearThreshold * sleepLinearThreshold
                   && abs(body.AngularVelocity) < sleepAngularThreshold;

if (belowThreshold) {
    body.SleepTimer += dt;
    if (body.SleepTimer >= sleepTimeThreshold) {  // default 0.5 seconds
        body.Sleep();
    }
} else {
    body.SleepTimer = 0.0f;  // any motion resets the timer
}
```

The velocity check uses squared magnitude to avoid a square root. The 0.5-second time threshold ensures bodies only sleep when they've genuinely settled, not when they're merely passing through a low-velocity state.

#### What Sleeping Does

When a body falls asleep:

```cpp
void Sleep() {
    bIsSleeping = true;
    Velocity = Vector2f(0, 0);
    AngularVelocity = 0.0f;
    ForceAccumulated = Vector2f(0, 0);
    TorqueAccumulated = 0.0f;
}
```

All velocities and forces are zeroed — the body is frozen in place. Sleeping bodies are then skipped by:

- **Integration** — no force-to-velocity or velocity-to-position updates
- **Damping** — no energy dissipation (there's no energy to dissipate)
- **Broadphase pair generation** — pairs where both bodies sleep are skipped

But sleeping bodies are *not* removed from the broadphase tree. Their fat AABB proxy stays in place so that moving bodies can still detect overlap with them and trigger a wake-up.

#### Waking Up: How Sleeping Bodies Rejoin the Simulation

A sleeping body wakes when something disturbs it. UMBRA checks three wake conditions:

**1. Collision with an awake body:**

```cpp
if (bodyA.bIsSleeping && !bodyB.bIsSleeping && !bodyB.IsStatic()) {
    bodyA.Wake();
}
```

Only dynamic, awake bodies can wake sleepers. Static bodies can't — a box resting on a static floor shouldn't wake every frame from the persistent floor contact. This creates natural cascading wake-ups: a ball hitting a stack of sleeping boxes wakes the impacted box, which collides with the box above it, waking that one too, propagating up the stack.

**2. Constraint partner is awake:** If a joint connects a sleeping body to an awake body, the sleeper wakes. A spring attached to a moving platform pulls its connected body back into the simulation.

**3. External force application:** Any call to `ApplyForce`, `ApplyImpulse`, or `ApplyTorque` wakes the body first:

```cpp
void ApplyForce(BodyHandle handle, Vector2f force) {
    if (force.SquareMagnitude() > 0.0f) {
        body->Wake();
    }
    body->ForceAccumulated += force;
}
```

This ensures gameplay systems (explosions, player input, AI) can always affect sleeping bodies without special-casing.

#### The `bCanSleep` Opt-Out

Bodies can disable sleeping entirely with `bCanSleep = false`. This is useful for the player character (which should always respond instantly to input), cameras, and any body that must remain responsive regardless of its velocity.

---

### Collision Events: Enter, Stay, Exit

The physics engine doesn't just resolve collisions — it also reports them to gameplay systems. UMBRA tracks collision pairs across frames to generate three event types:

- **Enter**: a pair that wasn't colliding last frame but is now
- **Stay**: a pair that was colliding last frame and still is
- **Exit**: a pair that was colliding last frame but no longer is

Each pair is identified by a 64-bit key combining both body indices. After the narrow phase, the current frame's pair set is compared against the previous frame's set:

- Pairs in both sets → **Stay**
- Pairs only in the current set → **Enter**
- Pairs only in the previous set → **Exit**

**Trigger bodies** (bodies marked as sensors) go through the same detection pipeline — broadphase, narrow phase, event categorization — but skip the contact solver entirely. They detect overlap without producing any physical response, which is perfect for pickup zones, damage areas, and other gameplay triggers.

---

### The Complete Simulation Step

Here is the full `PhysicsService::Step()` pipeline with every system in context:

```
 1. ApplySpringForces          (Hooke's law + damping, force-based)
 2. IntegrateForces            (forces → velocities via Velocity Verlet; skip sleeping)
 3. SaveCCDState               (snapshot positions for fast bodies)
 4. IntegrateVelocities        (velocities → positions via Verlet; skip sleeping)
 5. ApplyDamping               (exponential velocity decay; skip sleeping)
 6. ClearForceAccumulators     (reset forces/torques for next frame)
 7. PerformCCD                 (sweep fast bodies, clamp to earliest TOI)
 8. UpdateBroadphaseProxies    (move fat AABBs, reinsert if escaped)
 9. BroadphaseDetection        (tree queries → candidate pairs; filter layers + sleeping)
10. NarrowPhaseDetection       (SAT/clipping/circle tests on candidate pairs)
11. PrecomputeContactConstraints   (effective masses, lever arms, restitution bias)
12. PrecomputeConstraints          (joint effective masses, Baumgarte bias)
13. ResolveContacts × 8 + SolveConstraintVelocities × 8   (sequential impulse)
14. PositionContraction × 3 + SolveConstraintPositions × 3 (Baumgarte correction)
15. UpdateSleepingBodies       (wake on contact/constraint, sleep timer, freeze)
16. CategorizeCollisionEvents  (enter/stay/exit for collisions and triggers)
```

Each stage builds on the previous. Integration produces positions; CCD corrects tunneling; the broadphase tree prunes impossible pairs; the narrow phase computes contact geometry; the solver turns geometry into impulses; and the sleeping system identifies what can be skipped next frame.

The broadphase and sleeping systems together mean that a scene with 500 bodies — 480 of which are resting in settled stacks — only runs narrow-phase collision and solver iterations on the ~20 moving bodies and their immediate neighbors. The difference between O(n²) brute-force and this pipeline is often 100x or more in practice.

---

*That wraps up the 4-part series on UMBRA's physics engine. From the differential equations that define motion, through the numerical integrators that discretize them, the collision algorithms that detect intersection, the constraint solver that enforces physical laws, and finally the spatial and temporal optimizations that make it all run in real time — each layer builds on the last to turn mathematical constraints into convincing physical behavior.*
