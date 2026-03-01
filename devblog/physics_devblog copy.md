# Building a 2D Physics Engine from Scratch — A 4-Part Dev Blog

*A technical deep-dive into UMBRA Engine's `PhysicsService`: from Newton's laws to sleeping bodies.*

---

## Part 1 — Kinematic Integration: Teaching Bodies to Move

Every physics engine begins with the same question: *given the forces acting on a body right now, where will it be next frame?* This is the domain of **numerical integration** — turning continuous physics into discrete timesteps.

### The Simulation Loop

Each call to `PhysicsService::Step()` runs through a strict pipeline:

```
1. IntegrateForces     →  forces become velocities
2. IntegrateVelocities →  velocities become positions
3. ApplyDamping        →  bleed off energy
4. ClearAccumulators   →  reset for next frame
5-11. Collision & solving (Parts 2-4)
```

Splitting integration into two phases — forces first, then positions — is deliberate. It's the heart of the **Velocity Verlet** integrator.

### Why Velocity Verlet over Euler?

The naive Euler approach (`v += a*dt`, `p += v*dt`) is first-order: it accumulates drift quickly, especially under stiff forces like springs. **Velocity Verlet** is a symplectic, second-order integrator that preserves energy far better over long simulations, and it costs almost nothing extra.

The key insight: instead of using only the *current* acceleration to update velocity, Verlet averages the *old* and *new* accelerations:

```
v(t+dt) = v(t) + 0.5 * (a(t) + a(t+dt)) * dt
```

In code, `IntegrateForces` does exactly this:

```cpp
// Calculate new acceleration from accumulated forces
Vector2f newAcceleration = body.ForceAccumulated * body.InverseMass;
float newAngularAcceleration = body.TorqueAccumulated * body.InverseInertia;

// Velocity Verlet: average old + new acceleration
body.Velocity += (body.Acceleration + newAcceleration) * 0.5f * dt;
body.AngularVelocity += (body.AngularAcceleration + newAngularAcceleration) * 0.5f * dt;

// Store for next frame's averaging
body.Acceleration = newAcceleration;
body.AngularAcceleration = newAngularAcceleration;
```

This requires storing `Acceleration` and `AngularAcceleration` on the body between frames — that's the tradeoff for second-order accuracy.

### Position Update (The Other Half of Verlet)

Position integration uses the full Verlet displacement formula:

```
p(t+dt) = p(t) + v(t)*dt + 0.5 * a(t) * dt²
```

```cpp
body.Position += body.Velocity * dt + body.Acceleration * (dt*dt * 0.5f);
body.Angle += body.AngularVelocity * dt + body.AngularAcceleration * (dt*dt * 0.5f);
```

The `0.5 * a * dt²` term is small per frame but matters when bodies accelerate quickly — it prevents the "spiraling" artifacts you'd see with plain Euler on orbital or spring systems.

Angles are also normalized to `[0, 2π)` after integration to prevent floating-point drift over many rotations.

### Gravity as a Force

Gravity isn't treated as a special acceleration hack — it's applied as a *force* proportional to mass:

```cpp
if (body.bAffectedByGravity && body.InverseMass > 0) {
    Vector2f gravityForce = Gravity / body.InverseMass;  // F = m * g
    body.ForceAccumulated += gravityForce;
}
```

Since the engine stores `InverseMass` (1/m), dividing gravity by inverse mass gives `m * g`. This ensures gravity accelerates all objects equally regardless of mass (as Galileo intended) while keeping it in the force pipeline, so force-based features like "gravity scale" or "zero-G zones" are trivial.

### Exponential Damping

Linear damping (`v *= 0.99`) is frame-rate dependent. UMBRA uses **exponential damping** instead:

```cpp
body.Velocity *= pow(damping, dt);
body.AngularVelocity *= pow(angularDamping, dt);
```

With `damping = 0.975`, at 100fps (dt=0.01) you get `0.975^0.01 ≈ 0.99975` per frame, and at 50fps (dt=0.02) you get `0.975^0.02 ≈ 0.99949` — the rate of energy loss over *wall-clock time* is identical regardless of frame rate. This is critical when using a fixed timestep, because even fixed steps can vary if the physics budget changes.

### Inverse Mass: The Static Body Trick

Every mass and inertia value is stored inverted:

```cpp
float InverseMass;     // 0 = infinite mass (static)
float InverseInertia;  // 0 = infinite inertia
```

This is the standard physics engine trick: a body with `InverseMass = 0` naturally has zero acceleration from any force (`a = F * 0 = 0`), making it immovable without any special-casing. Static bodies, kinematic bodies, and sleeping bodies all go through the same code paths — the inverse-mass just zeros out their contributions.

### Rotational Inertia Auto-Calculation

When creating a body, if the user doesn't provide a moment of inertia, UMBRA calculates it from the shape:

- **Circle (solid disk):** `I = ½mr²`
- **Rectangle (solid):** `I = (1/12) * m * (w² + h²)`

These are the standard textbook formulas for uniform-density 2D shapes. Getting inertia right is surprisingly important — too low and boxes spin like tops, too high and they feel "frozen" rotationally.

### The Force Pipeline

Forces are accumulated per-frame and cleared after integration:

```
ApplyForce(F)           → adds F to accumulator
ApplyForceAtPoint(F, p) → adds F + generates torque from lever arm
ApplyImpulse(J)         → directly changes velocity (J * inverseMass)
ApplyTorque(τ)          → adds τ to torque accumulator
```

The distinction between **forces** (accumulated, integrated over dt) and **impulses** (instantaneous velocity change) matters: collision response uses impulses, while gameplay systems use forces.

---

## Part 2 — Collision Detection: SAT, Circles, and Contact Points

Integration tells us where bodies *want* to go. Collision detection tells us where they *can't*. UMBRA handles three collision pairs: circle-circle, circle-OBB, and OBB-OBB.

### The Dispatch Table

`CollisionQuery::CheckCollision` routes to the right algorithm based on shape types:

```
Circle vs Circle  →  TestCircleCircle
Circle vs Box     →  TestCircleVsOBB  (normal flipped to maintain A→B convention)
Box    vs Circle  →  TestCircleVsOBB  (argument order swapped)
Box    vs Box     →  TestBoxVsBoxSAT  →  TestPolygonVsPolygonOverlapSAT
```

Every collision function fills a `CollisionDef` with:
- **contactNormal**: unit vector pointing from body A toward body B
- **penetration**: overlap depth along that normal
- **contacts[]**: one or more contact points with individual penetration depths

The A→B normal convention is enforced everywhere — when `TestCircleVsOBB` returns a normal pointing the wrong way, `CheckCollision` flips it. This consistency is critical for the solver in Part 3.

### Circle vs Circle

The simplest test. Two circles overlap when the distance between their centers is less than the sum of their radii:

```cpp
float distSq = (posB - posA).SquareMagnitude();
float radiiSum = radiusA + radiusB;
if (distSq > radiiSum * radiiSum) return false;  // early-out with squared distance
```

The contact normal is the normalized displacement vector between centers, and the contact point sits on A's surface toward B:

```cpp
contactNormal = displacement / dist;
contactPoint = posA + contactNormal * radiusA;
penetration = radiiSum - dist;
```

The degenerate case (coincident centers) uses an arbitrary axis `(1, 0)` to avoid division by zero.

### Circle vs OBB (Oriented Bounding Box)

This uses the **closest-point-on-OBB-edge** approach:

1. Find the point on the OBB boundary closest to the circle center (using `GetClosestPointOnOrientedBoundEdge`, which projects the circle center into the OBB's local space, clamps to the half-extents, and transforms back)
2. If the distance from that point to the circle center is less than the radius, they overlap
3. The contact point *is* that closest point on the OBB surface

```cpp
Vector2f pointOnOBB = GetClosestPointOnOrientedBoundEdge(bounds, angle, circlePos);
float distance = (circlePos - pointOnOBB).Magnitude();
if (distance > radius) return false;

penetration = radius - distance;
contactNormal = (circlePos - pointOnOBB) / distance;
contactPoint = pointOnOBB;
```

This handles all cases: circle touching a face, an edge, or a corner of the box.

### Box vs Box: The Separating Axis Theorem (SAT)

SAT is the workhorse of convex polygon collision. The theorem states:

> **Two convex shapes are separated if and only if there exists an axis along which their projections do not overlap.**

For two convex polygons, the only candidate separating axes are the **edge normals** of both shapes. For two boxes (4 edges each), that's 8 candidate axes — though parallel edges reduce this to 4 unique directions.

#### Step 1: Build World-Space Polygons

Each box is converted to 4 world-space vertices by rotating the local half-extents by the body's angle:

```cpp
vertices.emplace_back(pos + Vector2f( halfW, -halfH).GetRotated(angle));
vertices.emplace_back(pos + Vector2f( halfW,  halfH).GetRotated(angle));
vertices.emplace_back(pos + Vector2f(-halfW,  halfH).GetRotated(angle));
vertices.emplace_back(pos + Vector2f(-halfW, -halfH).GetRotated(angle));
```

#### Step 2: Test All Axes

For each candidate axis (edge normals from both polygons), project all vertices of both shapes onto that axis and check for overlap:

```cpp
for (Vector2f axis : allAxes) {
    Projection projA = shapeA.GetProjectionOntoAxis(axis);
    Projection projB = shapeB.GetProjectionOntoAxis(axis);

    // Gap found — shapes are separated
    if (projA.Min > projB.Max || projA.Max < projB.Min) {
        return false;
    }

    // Track axis of minimum overlap (MTV direction)
    float overlap = min(projA.Max, projB.Max) - max(projA.Min, projB.Min);
    if (overlap < minOverlap) {
        minOverlap = overlap;
        minAxis = axis;
    }
}
```

If no separating axis is found, the shapes are colliding. The axis with the **smallest overlap** is the **Minimum Translation Vector (MTV)** — the cheapest direction to push the shapes apart.

#### Step 3: Orient the Normal

The MTV axis might point from B→A. We fix this by checking against the center-to-center vector:

```cpp
Vector2f AB = shapeB.GetCenter() - shapeA.GetCenter();
if (Dot(AB, minAxis) < 0) {
    minAxis = -minAxis;
}
```

Now `contactNormal` reliably points from A toward B.

### Contact Point Generation: Sutherland-Hodgman Clipping

SAT gives us the collision normal and penetration depth, but the solver needs *contact points* — the actual spots where the shapes touch. UMBRA uses **Sutherland-Hodgman polygon clipping** to find them.

#### Finding the Best Edges

For each polygon, we find the vertex that projects farthest along the contact normal (the "support point"). The **best edge** is the edge adjacent to that vertex that is most perpendicular to the contact normal:

```cpp
// For each neighbor edge of the support vertex,
// pick the one whose direction is most perpendicular to the normal
float prevProj = Dot(prevEdgeDir, normal);
float nextProj = Dot(nextEdgeDir, normal);
if (abs(prevProj) <= abs(nextProj))
    bestEdge = {prevVertex, supportVertex};
else
    bestEdge = {supportVertex, nextVertex};
```

#### Reference vs Incident Edge

The edge that is *more perpendicular* to the contact normal becomes the **reference edge** (the clipping boundary). The other is the **incident edge** (to be clipped):

```cpp
float e1Dot = abs(Dot(bestEdgeA_dir, normal));
float e2Dot = abs(Dot(bestEdgeB_dir, normal));
if (e1Dot <= e2Dot) {
    referenceEdge = bestEdgeA;  // more perpendicular
    incidentEdge = bestEdgeB;
} else {
    referenceEdge = bestEdgeB;
    incidentEdge = bestEdgeA;
}
```

#### Clipping

The incident edge is clipped against the two side planes of the reference edge. Each clip trims the edge to the region that lies within the reference face's extent:

```cpp
// Clip against left side plane
Clip(refEdgeDir, incidentEdge, Dot(refEdgeDir, refEdge.first));
// Clip against right side plane
Clip(-refEdgeDir, incidentEdge, Dot(-refEdgeDir, refEdge.second));
```

The `Clip` function implements Sutherland-Hodgman for a single plane: it keeps points on the positive side and interpolates new points where the edge crosses the plane:

```cpp
if (dist1 * dist2 < 0.0f) {
    float t = dist1 / (dist1 - dist2);
    newPoint = edge.first + t * (edge.second - edge.first);
}
```

#### Final Filter

After clipping, any remaining points that are behind the reference face (negative depth) are valid contact points:

```cpp
float depth = Dot(refNormal, point) - Dot(refNormal, refEdge.first);
if (depth <= 0.0f) {
    contact.contactPoint = point;
    contact.penetration = -depth;
    contacts.push_back(contact);
}
```

This typically yields 1 contact point for vertex-face collisions and 2 contact points for face-face (edge-edge in 2D) collisions. Having two contact points is crucial for stable stacking — a single point can't resist rotation.

---

## Part 3 — Contact Resolution: Sequential Impulse and Constraint Solving

We now know *where* bodies collide and *how deep* they overlap. The solver's job: compute impulses that prevent penetration, simulate friction, and do it all stably enough for boxes to stack.

### The Sequential Impulse Method

UMBRA uses **Sequential Impulse** (SI), the same algorithm powering Box2D and most real-time physics engines. The idea:

1. **Precompute** constraint data once per frame (effective masses, bias terms)
2. **Iterate** over all contacts multiple times, applying small corrective impulses
3. Each iteration improves the solution — with enough iterations, the system converges

This is essentially **Gauss-Seidel** iteration applied to the complementarity problem of contact constraints.

### Precomputation: `PrecomputeContactConstraints()`

Before the solver iterates, we compute everything that doesn't change between iterations:

#### Lever Arms

```cpp
contact.rA = contactPoint - bodyA.Position;
contact.rB = contactPoint - bodyB.Position;
```

These vectors from body centers to the contact point determine how impulses create torque.

#### Effective Mass (Normal)

The "effective mass" along the contact normal accounts for both bodies' masses and the rotational coupling through the lever arms:

```
1/m_eff = 1/mA + 1/mB + (rA × n)²/IA + (rB × n)²/IB
```

```cpp
float rACrossN = Cross2D(rA, normal);
float rBCrossN = Cross2D(rB, normal);
float normalDenom = invMassSum
    + rACrossN * rACrossN * invInertiaA
    + rBCrossN * rBCrossN * invInertiaB;
contact.normalMass = 1.0f / normalDenom;
```

The `(rA × n)²` terms represent how much of a normal impulse at the contact point goes into *rotation* rather than translation. A contact far from the center of mass is "softer" because the impulse is partly absorbed by angular acceleration.

#### Effective Mass (Tangent)

The exact same formula but with the tangent direction (perpendicular to the normal):

```cpp
Vector2f tangent(-normal.y, normal.x);
float rACrossT = Cross2D(rA, tangent);
// ... same pattern
contact.tangentMass = 1.0f / tangentDenom;
```

#### Restitution Velocity Bias

For bouncy collisions, we need to add energy. The bias is computed from the closing speed and the coefficient of restitution:

```cpp
Vector2f velA = bodyA.Velocity + Vector2f(-rA.y, rA.x) * bodyA.AngularVelocity;
Vector2f velB = bodyB.Velocity + Vector2f(-rB.y, rB.x) * bodyB.AngularVelocity;
float closingSpeed = Dot(velB - velA, normal);
contact.velocityBias = closingSpeed < -1.0f ? -e * closingSpeed : 0.0f;
```

The `-1.0f` threshold prevents micro-bounces at rest — if the closing speed is tiny, we just absorb it completely. This is a critical stability feature; without it, resting contacts jitter endlessly.

### The Velocity Solver: `ResolveContacts()`

This runs `VelocityIterations` times (default: 8). Each iteration loops over every contact and does two things:

#### Normal Impulse with Accumulated Clamping

```cpp
// Relative velocity at contact point
Vector2f relVel = velB - velA;
float velAlongNormal = Dot(relVel, normal);

// Delta impulse this iteration
float dj = (-velAlongNormal + velocityBias) * normalMass;

// Accumulated clamping (the key to SI stability)
float oldAccum = contact.normalImpulseAccum;
contact.normalImpulseAccum = max(oldAccum + dj, 0.0f);
dj = contact.normalImpulseAccum - oldAccum;  // actual delta to apply

// Apply
bodyA.Velocity -= normal * dj * invMassA;
bodyA.AngularVelocity -= Cross2D(rA, normal * dj) * invInertiaA;
bodyB.Velocity += normal * dj * invMassB;
bodyB.AngularVelocity += Cross2D(rB, normal * dj) * invInertiaB;
```

**Why accumulated clamping instead of per-iteration clamping?**

Per-iteration clamping (`dj = max(dj, 0)`) can only *add* normal impulse. But when multiple contacts share bodies, correcting one contact can *over-correct* another. With accumulated clamping, the total impulse is clamped (`max(total, 0)`), allowing the delta to be *negative* — pulling back impulse that was applied in a previous iteration. This converges to the correct solution much faster.

#### Friction Impulse with Coulomb Clamping

After the normal impulse updates velocities, we recompute relative velocity and apply friction along the fixed tangent:

```cpp
float velAlongTangent = Dot(relVel, tangent);
float djt = -velAlongTangent * tangentMass;

// Coulomb friction: |friction| <= mu * normalForce
float maxFriction = friction * contact.normalImpulseAccum;
float oldTangentAccum = contact.tangentImpulseAccum;
contact.tangentImpulseAccum = clamp(oldTangentAccum + djt, -maxFriction, maxFriction);
djt = contact.tangentImpulseAccum - oldTangentAccum;
```

The **Coulomb friction model** limits the tangential impulse to `mu * normal_impulse`. If the relative tangential velocity can be zeroed without exceeding this limit, it's **static friction** (the objects stick). If the limit is hit, it's **dynamic friction** (the objects slide). The accumulated clamping ensures both limits cooperate correctly across iterations.

The friction coefficient is the **geometric mean** of both bodies' friction values:

```cpp
float friction = sqrt(bodyA.DynamicFriction * bodyB.DynamicFriction);
```

Geometric mean (rather than arithmetic mean or min) means that a zero-friction surface is truly frictionless, while two high-friction surfaces are grippier than either alone.

### Position Correction: `PositionContraction()`

The velocity solver prevents *future* penetration, but bodies may already be overlapping from the current frame. **Position correction** directly nudges bodies apart:

```cpp
const float percent = 0.4f;  // correct 40% per iteration
const float slop = 0.01f;    // allow 1cm of overlap before correcting

float correctionMag = max(penetration - slop, 0.0f);
Vector2f correction = normal * (correctionMag / invMassSum) * percent;

bodyA.Position -= correction * invMassA;
bodyB.Position += correction * invMassB;
```

Two important stability parameters:

- **Slop** (penetration allowance): without it, the solver fights numerical drift every frame, causing visible jitter. The 1cm slop means contacts are allowed to slightly overlap, which is invisible at normal scale.

- **Correction percentage** (0.4): correcting 100% in one shot causes oscillation. Correcting a fraction and iterating (`PositionIterations = 3`) converges smoothly. This is essentially **Baumgarte stabilization** applied as direct position correction rather than velocity bias.

The correction is split between bodies proportional to their inverse masses — lighter bodies move more, and infinite-mass (static) bodies don't move at all.

### Sleeping Body Treatment

Throughout the solver, sleeping bodies are treated as having `InverseMass = 0` and `InverseInertia = 0`:

```cpp
float invMassA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
```

This means a sleeping body acts like a static body in the constraint solver — it participates in collisions (pushing dynamic bodies away) but doesn't move itself. This is cheaper than a full wake-up and prevents cascading wake-ups from minor contacts.

### Why Multiple Iterations?

A single pass through all contacts produces a valid solution for each contact *in isolation*, but contacts share bodies. Fixing contact 1 might violate contact 2. Each additional iteration reduces the remaining error exponentially.

The default 8 velocity iterations is a practical sweet spot: stable stacking of 5-10 objects, responsive collisions, and no visible jitter. More iterations improve stability at the cost of CPU time — it's a tunable quality knob.

---

## Part 4 — Broadphase, Dynamic AABB Tree, and Sleeping

With potentially hundreds of bodies, testing every pair for collisions is O(n²) — unacceptable. The broadphase and sleeping systems exist to make the engine scale.

### The Broadphase Problem

If you have 200 bodies, that's 19,900 potential collision pairs. Most of these are nowhere near each other. The **broadphase** cheaply eliminates pairs that can't possibly collide, leaving only a handful for the expensive narrow-phase (SAT, clipping) from Part 2.

### Dynamic AABB Tree

UMBRA uses a **Dynamic AABB Tree** — the same data structure Box2D uses. It's a binary tree where:

- **Leaf nodes** store a fattened AABB for each physics body
- **Internal nodes** store the union (bounding AABB) of their children
- The tree is kept balanced via AVL-style rotations

#### Why Not a Grid?

Uniform grids are simpler but struggle with varying body sizes and large worlds. Quadtrees handle spatial variance but are expensive to rebuild. The dynamic AABB tree is *incrementally maintained* — bodies are inserted, removed, and moved without rebuilding, making it ideal for real-time simulation.

#### Fat AABBs: Avoiding Constant Rebuilds

A body's tight AABB changes every frame as it moves. Rebuilding the tree every frame would negate its benefits. The solution: **fat AABBs**.

When a body is inserted, its AABB is *fattened* — expanded by a uniform margin plus a velocity-based prediction:

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

During `UpdateBroadphaseProxies`, if a body's tight AABB still fits inside its fat AABB, *the tree is not touched*. Only when a body moves enough to escape its fat envelope is the proxy re-inserted:

```cpp
bool MoveProxy(int32 proxyId, const Bounds2D& newAabb, const Vector2f& displacement) {
    // Still fits? Skip.
    if (nodes[proxyId].Aabb.Contains(newAabb)) return false;

    RemoveLeaf(proxyId);
    nodes[proxyId].Aabb = FattenAABB(newAabb, displacement);
    InsertLeaf(proxyId);
    return true;
}
```

The displacement multiplier (default 2x) means the fat AABB extends in the direction the body is heading, so a smoothly-moving body might not trigger a tree update for many frames.

#### Insertion: Surface Area Heuristic (SAH)

When inserting a leaf, we need to find the best sibling — the existing node that minimizes the total surface area increase in the tree. Lower total SA means tighter bounding volumes and fewer false positives during queries.

The algorithm uses a **cost-based tree traversal** (Box2D style):

```cpp
// For each candidate sibling, compute:
//   totalCost = SA(union(candidate, newLeaf)) + inheritedCost

// inheritedCost = sum of SA increases at all ancestors
// Lower bound for any descendant = SA(newLeaf) + inheritedCost

// Prune: if lowerBound >= bestCost, skip this subtree entirely
```

This is effectively a branch-and-bound search that finds the optimal sibling in O(log n) for reasonably balanced trees, compared to O(n) for a naive search.

#### AVL Balancing

After insertion or removal, the tree walks upward, fixing heights and AABBs, and applying AVL-style rotations when the balance factor exceeds 1:

```cpp
int32 balance = nodes[rightId].Height - nodes[leftId].Height;

if (balance > 1) {
    // Right-heavy: rotate right child up
    // Pick the taller grandchild to remain at the higher level
    // to minimize resulting height
}
if (balance < -1) {
    // Left-heavy: rotate left child up
}
```

The rotation picks the *taller* grandchild to stay higher in the tree, minimizing the resulting maximum height. This keeps the tree at O(log n) depth, guaranteeing O(log n) query performance.

#### Querying the Tree

Broadphase detection queries the tree for each body's fat AABB:

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

            overlappingPairs.emplace_back(handleA, handleB);
        });
    }
}
```

The tree query is stack-based (no recursion, no allocation):

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
                callback(nodeId);
            else {
                stack.push(node.Left);
                stack.push(node.Right);
            }
        }
    }
}
```

Branches whose AABB doesn't intersect the query are pruned entirely — in practice, this visits O(log n + k) nodes where k is the number of actual overlaps.

### Node Management: Pool Allocator

The tree uses a **free-list pool** for node allocation, avoiding heap allocation during gameplay:

```cpp
// Pre-allocate nodes with a linked free list
for (uint32 i = 0; i < capacity - 1; ++i) {
    nodes[i].NextFree = i + 1;
}

// Allocate: pop from free list
int32 AllocateNode() {
    if (freeList == NullNode) { /* double capacity, rebuild free list */ }
    int32 nodeId = freeList;
    freeList = nodes[nodeId].NextFree;
    return nodeId;
}

// Free: push to free list
void FreeNode(int32 nodeId) {
    nodes[nodeId].NextFree = freeList;
    freeList = nodeId;
}
```

When the pool runs out, it doubles in size — amortized O(1) allocation with zero per-frame heap allocations during steady state.

### Body Sleeping

Once a stack of boxes settles, there's no reason to keep simulating them. The **sleeping system** detects bodies at rest and skips them entirely.

#### Sleep Criteria

A body can sleep when both its linear and angular velocities are below thresholds for a sustained period:

```cpp
bool shouldSleep = body.Velocity.SquareMagnitude() < sleepLinearThreshold²
                && abs(body.AngularVelocity) < sleepAngularThreshold;

if (shouldSleep) {
    body.SleepTimer += dt;
    if (body.SleepTimer >= sleepTimeThreshold) {  // default 0.5 seconds
        body.Sleep();
    }
} else {
    body.SleepTimer = 0.0f;  // reset timer if moving
}
```

The time threshold (0.5s default) prevents premature sleeping — a ball at the apex of its arc has zero velocity momentarily, but the timer resets as soon as it starts falling again.

#### What Happens When a Body Sleeps

```cpp
void Sleep() {
    bIsSleeping = true;
    Velocity = Vector2f(0, 0);
    AngularVelocity = 0.0f;
    ForceAccumulated = Vector2f(0, 0);
    TorqueAccumulated = 0.0f;
}
```

Velocity and forces are zeroed — the body is frozen in place. Sleeping bodies are skipped by:
- Integration (forces, velocities)
- Damping
- But **not** by broadphase (their proxy stays in the tree)

#### Wake-Up: Contact Propagation

When an awake body collides with a sleeping body, the sleeper wakes up:

```cpp
for (each collision) {
    if (bodyA.bIsSleeping && !bodyB.bIsSleeping && !bodyB.IsStatic()) {
        bodyA.Wake();  // resets bIsSleeping and SleepTimer
    }
    if (bodyB.bIsSleeping && !bodyA.bIsSleeping && !bodyA.IsStatic()) {
        bodyB.Wake();
    }
}
```

Only dynamic awake bodies can wake sleepers — static bodies can't (a box resting on the floor shouldn't wake every frame). This creates natural cascading wake-ups: throw a ball at a stack of sleeping boxes, and only the impacted boxes wake up, which wake the ones above them, and so on.

Force application also triggers wake-ups:

```cpp
void ApplyForce(BodyHandle handle, Vector2f force) {
    if (force.SquareMagnitude() > 0.0f) {
        body->Wake();
    }
    body->ForceAccumulated += force;
}
```

#### Sleeping Bodies in the Solver

During constraint solving, sleeping bodies are treated as immovable:

```cpp
float invMassA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
```

This prevents the solver from moving sleeping bodies while still allowing them to provide correct support forces to awake bodies resting on them.

#### The Per-Body Sleep Flag

Bodies can opt out of sleeping with `bCanSleep = false` — useful for the player character, cameras, or anything that should always be responsive to gameplay input.

### Putting It All Together

Here's the full `Step()` flow with broadphase and sleeping in context:

```
 1. IntegrateForces           (skip sleeping)
 2. IntegrateVelocities       (skip sleeping)
 3. ApplyDamping              (skip sleeping)
 4. ClearForceAccumulators    (all active)
 5. UpdateBroadphaseProxies   (move fat AABBs)
 6. BroadphaseDetection       (tree queries, skip sleeping pairs)
 7. NarrowPhaseDetection      (SAT/clipping on broadphase pairs)
 8. PrecomputeConstraints     (effective masses, sleeping→invMass=0)
 9. ResolveContacts × 8       (sequential impulse, accumulated clamping)
10. PositionCorrection × 3    (Baumgarte-style nudge)
11. UpdateSleepingBodies      (wake on contact, sleep timer, freeze)
```

The broadphase tree and sleeping system together mean that a scene with 500 bodies — 480 of which are resting in settled stacks — only does narrow-phase collision on the ~20 moving bodies and their immediate neighbors. The difference between O(n²) brute-force and this pipeline is often 100x or more in practice.

---

*That wraps up the 4-part series on UMBRA's physics engine. From Velocity Verlet integration through SAT collision, sequential impulse solving, and finally the dynamic AABB tree with body sleeping — each layer builds on the last to turn a handful of mathematical constraints into convincing, real-time physical behavior.*
