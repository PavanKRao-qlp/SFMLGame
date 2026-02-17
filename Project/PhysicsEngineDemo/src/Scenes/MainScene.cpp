#include "MainScene.h"

#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/Constraint.h"
#include "Service/Physics/PhysicsBody.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

// ─── Helper: create a static box entity ──────────────────────────────────────
static Umbra::EntityID CreateStaticBox(Umbra::World* _world, Umbra::Math::Vector2f _pos, Umbra::Math::Vector2f _size,
    float _angle = 0.0f, float _restitution = 0.3f) {
    Umbra::EntityID e = _world->CreateEntity();
    _world->AddComponent<Umbra::TransformComponent>(e, _pos, _size, _angle);

    Umbra::RigidbodyHandleComponent rb;
    rb.Mass               = 0.0f;
    rb.bAffectedByGravity = false;
    rb.CoefOfRestitution  = _restitution;
    _world->AddComponent<Umbra::RigidbodyHandleComponent>(e, rb);
    _world->AddComponent<Umbra::BoxColliderComponent>(e, _size);
    return e;
}

// ─── Helper: create a dynamic box entity ─────────────────────────────────────
static Umbra::EntityID CreateDynamicBox(Umbra::World* _world, Umbra::Math::Vector2f _pos, Umbra::Math::Vector2f _size,
    float _mass = 1.0f, float _restitution = 0.4f, float _angle = 0.0f) {
    Umbra::EntityID e = _world->CreateEntity();
    _world->AddComponent<Umbra::TransformComponent>(e, _pos, _size, _angle);

    Umbra::RigidbodyHandleComponent rb;
    rb.Mass               = _mass;
    rb.bAffectedByGravity = true;
    rb.CoefOfRestitution  = _restitution;
    rb.LinearDamping      = 0.05f;
    rb.AngularDamping     = 0.05f;
    _world->AddComponent<Umbra::RigidbodyHandleComponent>(e, rb);
    _world->AddComponent<Umbra::BoxColliderComponent>(e, _size);
    return e;
}

// ─── Helper: create a dynamic circle entity ──────────────────────────────────
static Umbra::EntityID CreateDynamicCircle(Umbra::World* _world, Umbra::Math::Vector2f _pos, float _radius,
    float _mass = 1.0f, float _restitution = 0.5f, float _linearDamp = 0.05f) {
    Umbra::EntityID e = _world->CreateEntity();
    Umbra::Math::Vector2f size(_radius * 2.0f, _radius * 2.0f);
    _world->AddComponent<Umbra::TransformComponent>(e, _pos, size);

    Umbra::RigidbodyHandleComponent rb;
    rb.Mass               = _mass;
    rb.bAffectedByGravity = true;
    rb.CoefOfRestitution  = _restitution;
    rb.LinearDamping      = _linearDamp;
    rb.AngularDamping     = 0.05f;
    _world->AddComponent<Umbra::RigidbodyHandleComponent>(e, rb);

    Umbra::CircleColliderComponent circle;
    circle.Radius = _radius;
    _world->AddComponent<Umbra::CircleColliderComponent>(e, circle);
    return e;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Scene lifecycle
// ═════════════════════════════════════════════════════════════════════════════

void MainScene::Initialize() {
    CreateDefaultCamera(250.0f);
}

void MainScene::OnBeginPlay() {
    SetupArena();
    // SetupSpringChain();
    // SetupWindmill();
    //  SetupNewtonCradle();
    //  SetupRamp();
    // SetupTower();
    // SetupLooseBodies();
    // bNeedsConstraintSetup = true;
    // bNeedsInitialVelocity = true;
}

void MainScene::OnEndPlay() {
    CleanupPlayground();
}

void MainScene::OnFixedUpdate() {
    if (bNeedsConstraintSetup) {
        CreateConstraints();
    }

    if (bNeedsInitialVelocity && !bNeedsConstraintSetup) {
        // Give initial velocities to loose bodies and cradle trigger
        Umbra::World* world            = GetWorld();
        Umbra::PhysicsService* physics = world->GetPhysicsService();

        // Kick the leftmost cradle ball to get things moving
        if (mCradleBallStartIdx >= 0) {
            auto* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[mCradleBallStartIdx]);
            if (rb && rb->Handle.IsValid()) {
                physics->SetVelocity(rb->Handle, {-80.0f, 0.0f});
            }
        }

        // Give some loose bodies random-ish initial velocities
        for (int i = 0; i < mLooseBodyCount; ++i) {
            int idx  = mLooseBodyStartIdx + i;
            auto* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[idx]);
            if (rb && rb->Handle.IsValid()) {
                float vx = (i % 2 == 0) ? 40.0f + i * 10.0f : -30.0f - i * 8.0f;
                float vy = 20.0f + i * 15.0f;
                physics->SetVelocity(rb->Handle, {vx, vy});
            }
        }

        bNeedsInitialVelocity = false;
    }
}

void MainScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }

    // ── Scene navigation menu ────────────────────────────────
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Physics Engine Demo");
    if (ImGui::Button("Particle Object : Integration")) {
        GetSceneManager().GoToScene("ParticleIntegration");
    }
    if (ImGui::Button("Narrow Phase Collisions")) {
        GetSceneManager().GoToScene("NarrowPhase");
    }
    if (ImGui::Button("Broadphase AABB Tree")) {
        GetSceneManager().GoToScene("BroadphaseTree");
    }
    if (ImGui::Button("Sleep Demo")) {
        GetSceneManager().GoToScene("SleepDemo");
    }
    if (ImGui::Button("Collision Filter & CCD")) {
        GetSceneManager().GoToScene("FilterCCD");
    }
    if (ImGui::Button("Constraint Joints")) {
        GetSceneManager().GoToScene("ConstraintJoints");
    }
    ImGui::End();

    // ── Physics config widget ────────────────────────────────
    Umbra::PhysicsServiceConfig& config = GetWorld()->GetPhysicsService()->GetConfig();
    ImGui::SetNextWindowPos(ImVec2(270, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("Physics Config");

    ImGui::SeparatorText("Gravity & Damping");
    ImGui::DragFloat2("Gravity", &config.Gravity.x, 0.5f);
    ImGui::DragFloat("Linear Damping", &config.LinearDamping, 0.005f, 0.0f, 1.0f);
    ImGui::DragFloat("Angular Damping", &config.AngularDamping, 0.005f, 0.0f, 1.0f);

    ImGui::SeparatorText("Solver");
    ImGui::DragInt("Velocity Iterations", &config.VelocityIterations, 1, 1, 64);
    ImGui::DragInt("Position Iterations", &config.PositionIterations, 1, 1, 64);
    ImGui::DragFloat("Baumgarte Scale", &config.BaumgarteScale, 0.01f, 0.0f, 1.0f);

    ImGui::SeparatorText("Broadphase");
    ImGui::DragFloat("AABB Fatten Margin", &config.AABBFattenMargin, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("AABB Disp. Multiplier", &config.AABBDisplacementMultiplier, 0.1f, 0.0f, 10.0f);

    ImGui::SeparatorText("Sleep");
    ImGui::DragFloat("Linear Vel. Threshold", &config.LinearVelocitySleepThreshold, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Angular Vel. Threshold", &config.AngularVelocitySleepThreshold, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Sleep Time Threshold", &config.SleepTimeThreshold, 0.01f, 0.0f, 5.0f);

    ImGui::SeparatorText("CCD");
    ImGui::Checkbox("Enable CCD", &config.bEnableCCD);
    ImGui::DragFloat("CCD Motion Threshold", &config.CCDMotionThreshold, 0.01f, 0.0f, 5.0f);
    ImGui::DragInt("CCD Bisection Iterations", &config.CCDBoxBisectionIterations, 1, 1, 32);

    ImGui::SeparatorText("Memory");
    int capacity = static_cast<int>(config.InitialBodyCapacity);
    if (ImGui::DragInt("Initial Body Capacity", &capacity, 1, 1, 4096)) {
        config.InitialBodyCapacity = static_cast<uint32_t>(capacity);
    }

    ImGui::End();

    // ── Debug visuals for constraints ────────────────────────
    // DrawDebugVisuals();
}

Umbra::SharedPtr<Umbra::Scene> MainScene::InstantiateCopy() {
    return std::make_shared<MainScene>(*this);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Setup helpers
// ═════════════════════════════════════════════════════════════════════════════

void MainScene::SetupArena() {
    Umbra::World* world = GetWorld();

    // Floor
    mEntities.push_back(CreateStaticBox(world, {0.0f, -250.0f}, {700.0f, 20.0f}, 0.0f, 0.3f));

    // Left wall
    mEntities.push_back(CreateStaticBox(world, {-340.0f, 0.0f}, {20.0f, 520.0f}, 0.0f, 0.3f));

    // Right wall
    mEntities.push_back(CreateStaticBox(world, {340.0f, 0.0f}, {20.0f, 520.0f}, 0.0f, 0.3f));

    // Ceiling
    mEntities.push_back(CreateStaticBox(world, {0.0f, 260.0f}, {700.0f, 20.0f}, 0.0f, 0.3f));
}

void MainScene::SetupSpringChain() {
    Umbra::World* world = GetWorld();
    float radius        = 10.0f;
    float startY        = 180.0f;
    float spacing       = 30.0f;

    // SPRING_CHAIN_START should match mEntities.size() right now
    // Store the actual start index
    int actualStart = static_cast<int>(mEntities.size());

    for (int i = 0; i < SPRING_CHAIN_COUNT; ++i) {
        float y              = startY - i * spacing;
        Umbra::EntityID ball = CreateDynamicCircle(world, {mSpringWorldAnchor.x, y}, radius, 1.5f, 0.3f, 0.1f);
        mEntities.push_back(ball);
    }

    // Verify our constant matches (first chain ball should be at index SPRING_CHAIN_START if arena
    // has 4 entities)
    // We'll use actualStart for safety in constraint creation
    (void) actualStart;
}

void MainScene::SetupWindmill() {
    Umbra::World* world = GetWorld();

    // Static pivot body (tiny, just an anchor)
    mWindmillPivotIdx = static_cast<int>(mEntities.size());
    mEntities.push_back(CreateStaticBox(world, {0.0f, 160.0f}, {10.0f, 10.0f}));

    // Dynamic arm (long thin box)
    mWindmillArmIdx = static_cast<int>(mEntities.size());
    mEntities.push_back(CreateDynamicBox(world, {0.0f, 160.0f}, {140.0f, 8.0f}, 3.0f, 0.2f));
}

void MainScene::SetupNewtonCradle() {
    Umbra::World* world = GetWorld();
    float barX          = 170.0f;
    float barY          = 210.0f;
    float ballRadius    = 12.0f;
    float ballY         = 120.0f;
    float ballSpacing   = ballRadius * 2.0f + 1.0f; // tight spacing

    // Static bar
    mCradleBarIdx = static_cast<int>(mEntities.size());
    mEntities.push_back(CreateStaticBox(world, {barX, barY}, {130.0f, 8.0f}));

    // 5 cradle balls
    mCradleBallStartIdx = static_cast<int>(mEntities.size());
    float firstBallX    = barX - (CRADLE_BALL_COUNT - 1) * ballSpacing * 0.5f;
    for (int i = 0; i < CRADLE_BALL_COUNT; ++i) {
        float x              = firstBallX + i * ballSpacing;
        Umbra::EntityID ball = CreateDynamicCircle(world, {x, ballY}, ballRadius, 2.0f, 0.95f, 0.01f);
        mEntities.push_back(ball);
    }
}

void MainScene::SetupRamp() {
    Umbra::World* world = GetWorld();

    // Angled ramp (mid-left)
    mEntities.push_back(CreateStaticBox(world, {-150.0f, -120.0f}, {180.0f, 12.0f}, 25.0f, 0.2f));

    // Small platform to catch balls rolling off
    mEntities.push_back(CreateStaticBox(world, {-50.0f, -190.0f}, {80.0f, 10.0f}, 0.0f, 0.2f));

    // A few balls sitting on the ramp
    mEntities.push_back(CreateDynamicCircle(world, {-200.0f, -60.0f}, 10.0f, 1.0f, 0.4f));
    mEntities.push_back(CreateDynamicCircle(world, {-220.0f, -40.0f}, 8.0f, 0.8f, 0.4f));
    mEntities.push_back(CreateDynamicCircle(world, {-180.0f, -30.0f}, 12.0f, 1.2f, 0.4f));
}

void MainScene::SetupTower() {
    Umbra::World* world = GetWorld();
    float baseX         = 220.0f;
    float baseY         = -230.0f; // just above the floor
    float boxW          = 40.0f;
    float boxH          = 18.0f;
    int layers          = 5;

    for (int i = 0; i < layers; ++i) {
        float y    = baseY + i * (boxH + 1.0f);
        float xOff = (i % 2 == 0) ? 0.0f : 5.0f; // slight offset for instability
        mEntities.push_back(CreateDynamicBox(world, {baseX + xOff, y}, {boxW, boxH}, 1.5f, 0.2f));
    }
}

void MainScene::SetupLooseBodies() {
    Umbra::World* world = GetWorld();

    mLooseBodyStartIdx = static_cast<int>(mEntities.size());

    // Circles (left half of arena floor)
    mEntities.push_back(CreateDynamicCircle(world, {-250.0f, -200.0f}, 12.0f, 1.0f, 0.7f));
    mEntities.push_back(CreateDynamicCircle(world, {-210.0f, -190.0f}, 9.0f, 0.8f, 0.6f));
    mEntities.push_back(CreateDynamicCircle(world, {-170.0f, -210.0f}, 14.0f, 1.3f, 0.8f));
    mEntities.push_back(CreateDynamicCircle(world, {-130.0f, -195.0f}, 10.0f, 1.0f, 0.5f));
    mEntities.push_back(CreateDynamicCircle(world, {-90.0f, -205.0f}, 11.0f, 1.1f, 0.65f));
    mEntities.push_back(CreateDynamicCircle(world, {-50.0f, -200.0f}, 8.0f, 0.7f, 0.75f));

    // Boxes (right half of arena floor)
    mEntities.push_back(CreateDynamicBox(world, {50.0f, -200.0f}, {20.0f, 20.0f}, 1.0f, 0.4f));
    mEntities.push_back(CreateDynamicBox(world, {90.0f, -195.0f}, {25.0f, 15.0f}, 1.2f, 0.35f));
    mEntities.push_back(CreateDynamicBox(world, {130.0f, -205.0f}, {18.0f, 22.0f}, 0.9f, 0.45f));
    mEntities.push_back(CreateDynamicBox(world, {170.0f, -200.0f}, {22.0f, 18.0f}, 1.1f, 0.4f));

    mLooseBodyCount = static_cast<int>(mEntities.size()) - mLooseBodyStartIdx;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Deferred constraint creation
// ═════════════════════════════════════════════════════════════════════════════

void MainScene::CreateConstraints() {
    Umbra::World* world            = GetWorld();
    Umbra::PhysicsService* physics = world->GetPhysicsService();

    // Lambda to safely extract a BodyHandle from an entity index
    auto getHandle = [&](int _idx) -> Umbra::BodyHandle {
        if (_idx < 0 || _idx >= static_cast<int>(mEntities.size())) {
            return Umbra::BodyHandle::Invalid();
        }
        auto* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[_idx]);
        if (!rb || !rb->Handle.IsValid()) {
            return Umbra::BodyHandle::Invalid();
        }
        return rb->Handle;
    };

    // Check that all key handles are valid before proceeding
    // (Arena entities start at index 0; chain starts at index 4)
    int chainStart = 4; // 4 arena entities: floor, left wall, right wall, ceiling

    for (int i = 0; i < SPRING_CHAIN_COUNT; ++i) {
        if (!getHandle(chainStart + i).IsValid()) {
            return;
        }
    }
    if (!getHandle(mWindmillPivotIdx).IsValid() || !getHandle(mWindmillArmIdx).IsValid()) {
        return;
    }
    if (!getHandle(mCradleBarIdx).IsValid()) {
        return;
    }
    for (int i = 0; i < CRADLE_BALL_COUNT; ++i) {
        if (!getHandle(mCradleBallStartIdx + i).IsValid()) {
            return;
        }
    }

    bNeedsConstraintSetup = false;

    // ── Spring chain (world anchor → ball[0] → ball[1] → ... → ball[4]) ────
    {
        // First spring: world anchor to first ball
        Umbra::SpringDef def;
        def.BodyA        = getHandle(chainStart);
        def.BodyB        = Umbra::BodyHandle::Invalid(); // world anchor
        def.WorldAnchorB = mSpringWorldAnchor;
        def.Stiffness    = 120.0f;
        def.Damping      = 6.0f;
        def.RestLength   = 25.0f;
        mConstraints.push_back(physics->CreateSpring(def));

        // Subsequent springs: ball-to-ball
        for (int i = 0; i < SPRING_CHAIN_COUNT - 1; ++i) {
            Umbra::SpringDef link;
            link.BodyA      = getHandle(chainStart + i);
            link.BodyB      = getHandle(chainStart + i + 1);
            link.Stiffness  = 200.0f;
            link.Damping    = 5.0f;
            link.RestLength = 25.0f;
            mConstraints.push_back(physics->CreateSpring(link));
        }
    }

    // ── Windmill hinge with motor ────────────────────────────────────────────
    {
        Umbra::HingeDef def;
        def.BodyA          = getHandle(mWindmillPivotIdx);
        def.BodyB          = getHandle(mWindmillArmIdx);
        def.LocalAnchorA   = {0.0f, 0.0f};
        def.LocalAnchorB   = {0.0f, 0.0f};
        def.bEnableMotor   = true;
        def.MotorSpeed     = 3.0f;
        def.MaxMotorTorque = 800.0f;
        mConstraints.push_back(physics->CreateHinge(def));
    }

    // ── Newton's cradle: distance constraints from bar to each ball ─────────
    {
        Umbra::BodyHandle barHandle = getHandle(mCradleBarIdx);
        float barX                  = 170.0f;
        float ballSpacing           = 12.0f * 2.0f + 1.0f;
        float firstBallX            = barX - (CRADLE_BALL_COUNT - 1) * ballSpacing * 0.5f;

        for (int i = 0; i < CRADLE_BALL_COUNT; ++i) {
            float localX = firstBallX + i * ballSpacing - barX;

            Umbra::DistanceDef def;
            def.BodyA        = barHandle;
            def.BodyB        = getHandle(mCradleBallStartIdx + i);
            def.LocalAnchorA = {localX, 0.0f};
            def.LocalAnchorB = {0.0f, 0.0f};
            def.Distance     = 90.0f; // bar at y=210, balls at y=120 → ~90 units
            mConstraints.push_back(physics->CreateDistanceConstraint(def));
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Debug visualization
// ═════════════════════════════════════════════════════════════════════════════

void MainScene::DrawColliderOutlines() {
    Umbra::World* world            = GetWorld();
    Umbra::PhysicsService* physics = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        auto* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (!rb || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody = physics->GetBodyData(rb->Handle);
        auto* transform                  = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (!physBody || !transform) {
            continue;
        }

        Umbra::Color color = physBody->IsStatic() ? Umbra::Color(0.4f, 0.4f, 0.4f) : Umbra::Color::White;

        if (physBody->BodyShape.IsCircle()) {
            Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), false, color);
        } else if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::ServiceLocator::GetRenderService()->DebugDrawOrientedBox(bounds, transform->Angle, false, color);
        }
    }
}

void MainScene::DrawDebugVisuals() {
    if (mEntities.empty()) {
        return;
    }

    DrawColliderOutlines();

    Umbra::World* world            = GetWorld();
    Umbra::PhysicsService* physics = world->GetPhysicsService();
    int chainStart                 = 4;

    // ── Spring chain lines ───────────────────────────────────────────────────
    {
        // World anchor marker
        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(mSpringWorldAnchor, 5.0f, true, Umbra::Color::Yellow);

        // Line from anchor to first ball
        auto* rb0 = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[chainStart]);
        if (rb0 && rb0->Handle.IsValid()) {
            Umbra::Math::Vector2f pos0 = physics->GetPosition(rb0->Handle);
            Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(mSpringWorldAnchor, pos0, Umbra::Color::Yellow);
        }

        // Lines between chain links
        for (int i = 0; i < SPRING_CHAIN_COUNT - 1; ++i) {
            auto* rbA = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[chainStart + i]);
            auto* rbB = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[chainStart + i + 1]);
            if (rbA && rbA->Handle.IsValid() && rbB && rbB->Handle.IsValid()) {
                Umbra::Math::Vector2f posA = physics->GetPosition(rbA->Handle);
                Umbra::Math::Vector2f posB = physics->GetPosition(rbB->Handle);
                Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(posA, posB, Umbra::Color::Yellow);
            }
        }
    }

    // ── Newton's cradle distance lines ───────────────────────────────────────
    if (mCradleBarIdx >= 0 && mCradleBallStartIdx >= 0) {
        auto* barRb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[mCradleBarIdx]);
        if (barRb && barRb->Handle.IsValid()) {
            Umbra::Math::Vector2f barPos = physics->GetPosition(barRb->Handle);

            float barX        = 170.0f;
            float ballSpacing = 12.0f * 2.0f + 1.0f;
            float firstBallX  = barX - (CRADLE_BALL_COUNT - 1) * ballSpacing * 0.5f;

            for (int i = 0; i < CRADLE_BALL_COUNT; ++i) {
                auto* ballRb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[mCradleBallStartIdx + i]);
                if (ballRb && ballRb->Handle.IsValid()) {
                    float localX                  = firstBallX + i * ballSpacing - barX;
                    Umbra::Math::Vector2f top     = {barPos.x + localX, barPos.y};
                    Umbra::Math::Vector2f ballPos = physics->GetPosition(ballRb->Handle);
                    Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(top, ballPos, Umbra::Color::Cyan);
                }
            }
        }
    }

    // ── Windmill pivot marker ────────────────────────────────────────────────
    if (mWindmillPivotIdx >= 0) {
        auto* pivotRb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[mWindmillPivotIdx]);
        if (pivotRb && pivotRb->Handle.IsValid()) {
            Umbra::Math::Vector2f pivotPos = physics->GetPosition(pivotRb->Handle);
            Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pivotPos, 6.0f, true, Umbra::Color::Red);
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Cleanup
// ═════════════════════════════════════════════════════════════════════════════

void MainScene::CleanupPlayground() {
    Umbra::World* world            = GetWorld();
    Umbra::PhysicsService* physics = world->GetPhysicsService();

    for (auto& handle : mConstraints) {
        if (physics->IsConstraintValid(handle)) {
            physics->DestroyConstraint(handle);
        }
    }
    mConstraints.clear();

    for (auto entity : mEntities) {
        world->DestroyEntity(entity);
    }
    mEntities.clear();

    bNeedsConstraintSetup = false;
    bNeedsInitialVelocity = false;
    mWindmillPivotIdx     = -1;
    mWindmillArmIdx       = -1;
    mCradleBarIdx         = -1;
    mCradleBallStartIdx   = -1;
    mLooseBodyStartIdx    = -1;
    mLooseBodyCount       = 0;
}
