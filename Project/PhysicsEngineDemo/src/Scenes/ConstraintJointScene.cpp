#include "ConstraintJointScene.h"

#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Math/MathUtils.h"
#include "Service/Physics/Constraint.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

// ────────────────────────────────────────────────────────────────
// Scene lifecycle
// ────────────────────────────────────────────────────────────────

void ConstraintJointScene::Initialize() {
    CreateDefaultCamera(300.0f);
}

void ConstraintJointScene::OnFixedUpdate() {
    // Deferred constraint creation: body handles become valid after first physics tick
    if (bNeedsConstraintSetup) {
        CreateConstraints();
    }

    // Move chain head to mouse position
    if (!mChainBodyHandles.empty() && mChainBodyHandles[0].IsValid()) {
        Umbra::Math::Vector2f mouseWorld =
            GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
        GetWorld()->GetPhysicsService()->SetPosition(mChainBodyHandles[0], mouseWorld);
        GetWorld()->GetPhysicsService()->SetVelocity(mChainBodyHandles[0], Umbra::Math::Vector2f(0, 0));
    }
}

void ConstraintJointScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // ── ImGui Panel ──────────────────────────────────────────────
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 480), ImGuiCond_FirstUseEver);
    ImGui::Begin("Constraint Joints Demo");

    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();

    // ── Spring controls ──────────────────────────────────────────
    if (ImGui::CollapsingHeader("Spring (Yellow)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped(
            "A body hanging from a world-anchor spring. "
            "Gravity pulls it down, the spring pulls it back.");
        bool changed = false;
        changed |= ImGui::DragFloat("Stiffness", &mSpringStiffness, 1.0f, 1.0f, 500.0f);
        changed |= ImGui::DragFloat("Damping", &mSpringDamping, 0.1f, 0.0f, 50.0f);
        changed |= ImGui::DragFloat("Rest Length", &mSpringRestLength, 1.0f, 0.0f, 200.0f);

        if (changed && physicsService->IsConstraintValid(mSpringHandle)) {
            physicsService->SetSpringStiffness(mSpringHandle, mSpringStiffness);
            physicsService->SetSpringDamping(mSpringHandle, mSpringDamping);
            physicsService->SetSpringRestLength(mSpringHandle, mSpringRestLength);
        }
    }
    ImGui::Separator();

    // ── Distance controls ────────────────────────────────────────
    if (ImGui::CollapsingHeader("Distance (Cyan)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped(
            "Two bodies connected by a rigid distance constraint "
            "(pendulum). The top body is a static anchor.");
    }
    ImGui::Separator();

    // ── Hinge controls ──────────────────────────────────────────
    if (ImGui::CollapsingHeader("Hinge (Magenta)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped(
            "Two bodies pinned at a shared point with free rotation. "
            "Optional angle limits and motor.");
        bool hingeChanged = false;

        hingeChanged |= ImGui::Checkbox("Enable Limits", &bHingeEnableLimits);
        if (bHingeEnableLimits) {
            hingeChanged |= ImGui::DragFloat("Lower Angle", &mHingeLowerAngle, 0.01f, -Umbra::Math::PI, 0.0f);
            hingeChanged |= ImGui::DragFloat("Upper Angle", &mHingeUpperAngle, 0.01f, 0.0f, Umbra::Math::PI);
        }
        hingeChanged |= ImGui::Checkbox("Enable Motor", &bHingeEnableMotor);
        if (bHingeEnableMotor) {
            hingeChanged |= ImGui::DragFloat("Motor Speed", &mHingeMotorSpeed, 0.1f, -20.0f, 20.0f);
            hingeChanged |= ImGui::DragFloat("Max Torque", &mHingeMaxMotorTorque, 10.0f, 0.0f, 2000.0f);
        }

        if (hingeChanged && physicsService->IsConstraintValid(mHingeHandle)) {
            physicsService->SetHingeLimitsEnabled(mHingeHandle, bHingeEnableLimits);
            physicsService->SetHingeLimits(mHingeHandle, mHingeLowerAngle, mHingeUpperAngle);
            physicsService->SetHingeMotorEnabled(mHingeHandle, bHingeEnableMotor);
            physicsService->SetHingeMotorSpeed(mHingeHandle, mHingeMotorSpeed);
            physicsService->SetHingeMaxMotorTorque(mHingeHandle, mHingeMaxMotorTorque);
        }
    }
    ImGui::Separator();

    // ── Spring Chain controls ───────────────────────────────────
    if (ImGui::CollapsingHeader("Spring Chain (Green)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped("Chain of bodies connected by springs. Head follows mouse cursor.");
        ImGui::DragFloat("Chain Stiffness", &mChainStiffness, 5.0f, 10.0f, 2000.0f);
        ImGui::DragFloat("Chain Damping", &mChainDamping, 0.5f, 0.0f, 50.0f);
        ImGui::DragFloat("Chain Rest Length", &mChainRestLength, 1.0f, 0.0f, 100.0f);
    }
    ImGui::Separator();

    // ── Reset ────────────────────────────────────────────────────
    if (ImGui::Button("Reset Scene")) {
        CleanupScene();
        SetupEntities();
    }

    ImGui::Checkbox("Show Collider Outlines", &bShowColliders);

    ImGui::End();

    // ── Debug Drawing ────────────────────────────────────────────
    DrawConstraintLines();
    if (bShowColliders) {
        DrawColliderOutlines();
    }
}

Umbra::SharedPtr<Umbra::Scene> ConstraintJointScene::InstantiateCopy() {
    return std::make_shared<ConstraintJointScene>(*this);
}

void ConstraintJointScene::OnBeginPlay() {
    SetupEntities();
}

void ConstraintJointScene::OnEndPlay() {
    CleanupScene();
}

// ────────────────────────────────────────────────────────────────
// Entity creation
// ────────────────────────────────────────────────────────────────

void ConstraintJointScene::SetupEntities() {
    Umbra::World* world = GetWorld();

    mEntities.clear();
    mConstraints.clear();
    mSpringHandle   = Umbra::ConstraintHandle::Invalid();
    mDistanceHandle = Umbra::ConstraintHandle::Invalid();
    mHingeHandle    = Umbra::ConstraintHandle::Invalid();
    mSpringBodyHandle = Umbra::BodyHandle::Invalid();
    mDistanceBodyA    = Umbra::BodyHandle::Invalid();
    mDistanceBodyB    = Umbra::BodyHandle::Invalid();
    mHingeBodyA       = Umbra::BodyHandle::Invalid();
    mHingeBodyB       = Umbra::BodyHandle::Invalid();

    // ── [0] Floor ────────────────────────────────────────────────
    {
        Umbra::EntityID floor = world->CreateEntity();
        mEntities.push_back(floor);
        world->AddComponent<Umbra::TransformComponent>(
            floor, Umbra::Math::Vector2f(0.0f, -250.0f), Umbra::Math::Vector2f(600.0f, 20.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.3f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(floor, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(floor, Umbra::Math::Vector2f(600.0f, 20.0f));
    }

    // ================================================================
    // LEFT: Spring Demo (x ~ -180)
    // ================================================================
    mSpringWorldAnchor = Umbra::Math::Vector2f(-180.0f, 150.0f);

    // ── [1] Spring body ──────────────────────────────────────────
    {
        Umbra::EntityID springBody = world->CreateEntity();
        mEntities.push_back(springBody);

        float radius = 15.0f;
        world->AddComponent<Umbra::TransformComponent>(
            springBody, Umbra::Math::Vector2f(-180.0f, 50.0f), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 2.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = 0.3f;
        rb.LinearDamping      = 0.1f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(springBody, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(springBody, circle);
    }

    // ================================================================
    // CENTER: Distance Constraint Demo (x ~ 0)
    // ================================================================

    // ── [2] Distance anchor (static) ─────────────────────────────
    {
        Umbra::EntityID anchorBody = world->CreateEntity();
        mEntities.push_back(anchorBody);

        float anchorSize = 10.0f;
        world->AddComponent<Umbra::TransformComponent>(
            anchorBody, Umbra::Math::Vector2f(0.0f, 150.0f), Umbra::Math::Vector2f(anchorSize * 2, anchorSize * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(anchorBody, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = anchorSize;
        world->AddComponent<Umbra::CircleColliderComponent>(anchorBody, circle);
    }

    // ── [3] Distance pendulum (dynamic) ──────────────────────────
    {
        Umbra::EntityID pendulumBody = world->CreateEntity();
        mEntities.push_back(pendulumBody);

        float pendulumRadius = 15.0f;
        world->AddComponent<Umbra::TransformComponent>(
            pendulumBody,
            Umbra::Math::Vector2f(60.0f, 50.0f),
            Umbra::Math::Vector2f(pendulumRadius * 2, pendulumRadius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 2.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = 0.3f;
        rb.LinearDamping      = 0.05f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(pendulumBody, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = pendulumRadius;
        world->AddComponent<Umbra::CircleColliderComponent>(pendulumBody, circle);
    }

    // ================================================================
    // RIGHT: Hinge Joint Demo (x ~ 180)
    // ================================================================

    // ── [4] Hinge anchor (static) ────────────────────────────────
    {
        Umbra::EntityID hingeAnchor = world->CreateEntity();
        mEntities.push_back(hingeAnchor);

        float anchorSize = 10.0f;
        world->AddComponent<Umbra::TransformComponent>(
            hingeAnchor,
            Umbra::Math::Vector2f(180.0f, 100.0f),
            Umbra::Math::Vector2f(anchorSize * 2, anchorSize * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(hingeAnchor, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = anchorSize;
        world->AddComponent<Umbra::CircleColliderComponent>(hingeAnchor, circle);
    }

    // ── [5] Hinge arm (dynamic box) ─────────────────────────────
    {
        Umbra::EntityID hingeArm = world->CreateEntity();
        mEntities.push_back(hingeArm);

        float armWidth  = 80.0f;
        float armHeight = 15.0f;
        world->AddComponent<Umbra::TransformComponent>(
            hingeArm,
            Umbra::Math::Vector2f(220.0f, 100.0f),
            Umbra::Math::Vector2f(armWidth, armHeight));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 3.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = 0.2f;
        rb.AngularDamping     = 0.3f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(hingeArm, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(hingeArm, Umbra::Math::Vector2f(armWidth, armHeight));
    }

    // ================================================================
    // BOTTOM-CENTER: Spring Chain (mouse-attached)
    // ================================================================
    mChainStartIdx = (int)mEntities.size();
    mChainBodyHandles.clear();

    for (int i = 0; i < CHAIN_LINK_COUNT; ++i) {
        Umbra::EntityID link = world->CreateEntity();
        mEntities.push_back(link);

        float x = -50.0f + i * mChainSpacing;
        float y = -100.0f;
        world->AddComponent<Umbra::TransformComponent>(
            link,
            Umbra::Math::Vector2f(x, y),
            Umbra::Math::Vector2f(mChainRadius * 2, mChainRadius * 2));

        Umbra::RigidbodyHandleComponent rb;
        if (i == 0) {
            // Head: kinematic, follows mouse
            rb.Mass               = 1.0f;
            rb.bIsKinematic       = true;
            rb.bAffectedByGravity = false;
            rb.bCanSleep          = false;
        } else {
            rb.Mass               = 1.0f;
            rb.bAffectedByGravity = true;
            rb.LinearDamping      = 0.5f;
            rb.CoefOfRestitution  = 0.1f;
            rb.bCanSleep          = false;
        }
        world->AddComponent<Umbra::RigidbodyHandleComponent>(link, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = mChainRadius;
        world->AddComponent<Umbra::CircleColliderComponent>(link, circle);
    }

    bNeedsConstraintSetup = true;
}

// ────────────────────────────────────────────────────────────────
// Deferred constraint creation (called from OnFixedUpdate)
// ────────────────────────────────────────────────────────────────

void ConstraintJointScene::CreateConstraints() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // Check all body handles are valid before creating constraints
    auto getHandle = [&](int _idx) -> Umbra::BodyHandle {
        if (_idx >= (int)mEntities.size()) return Umbra::BodyHandle::Invalid();
        Umbra::RigidbodyHandleComponent* rb =
            world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[_idx]);
        if (rb == nullptr || !rb->Handle.IsValid()) return Umbra::BodyHandle::Invalid();
        return rb->Handle;
    };

    Umbra::BodyHandle springHandle   = getHandle(IDX_SPRING_BODY);
    Umbra::BodyHandle distAnchor     = getHandle(IDX_DIST_ANCHOR);
    Umbra::BodyHandle distPendulum   = getHandle(IDX_DIST_PENDULUM);
    Umbra::BodyHandle hingeAnchor    = getHandle(IDX_HINGE_ANCHOR);
    Umbra::BodyHandle hingeArm       = getHandle(IDX_HINGE_ARM);

    // Wait until all handles are valid
    if (!springHandle.IsValid() || !distAnchor.IsValid() || !distPendulum.IsValid() ||
        !hingeAnchor.IsValid() || !hingeArm.IsValid()) {
        return;
    }

    bNeedsConstraintSetup = false;

    // Cache handles for debug drawing
    mSpringBodyHandle = springHandle;
    mDistanceBodyA    = distAnchor;
    mDistanceBodyB    = distPendulum;
    mHingeBodyA       = hingeAnchor;
    mHingeBodyB       = hingeArm;

    // ── Spring: body to world anchor ─────────────────────────────
    {
        Umbra::SpringDef def;
        def.BodyA        = springHandle;
        def.BodyB        = Umbra::BodyHandle::Invalid();
        def.WorldAnchorB = mSpringWorldAnchor;
        def.Stiffness    = mSpringStiffness;
        def.Damping       = mSpringDamping;
        def.RestLength    = mSpringRestLength;

        mSpringHandle = physicsService->CreateSpring(def);
        mConstraints.push_back(mSpringHandle);
    }

    // ── Distance: static anchor to pendulum ──────────────────────
    {
        Umbra::DistanceDef def;
        def.BodyA = distAnchor;
        def.BodyB = distPendulum;
        // Distance auto-calculated from initial positions (default 0)

        mDistanceHandle = physicsService->CreateDistanceConstraint(def);
        mConstraints.push_back(mDistanceHandle);
    }

    // ── Hinge: static anchor to arm ──────────────────────────────
    {
        Umbra::HingeDef def;
        def.BodyA = hingeAnchor;
        def.BodyB = hingeArm;
        // Local anchors: anchor body center to arm's left edge
        def.LocalAnchorA    = Umbra::Math::Vector2f(0.0f, 0.0f);
        def.LocalAnchorB    = Umbra::Math::Vector2f(-40.0f, 0.0f); // half arm width to the left
        def.bEnableLimits   = bHingeEnableLimits;
        def.LowerAngle      = mHingeLowerAngle;
        def.UpperAngle       = mHingeUpperAngle;
        def.bEnableMotor     = bHingeEnableMotor;
        def.MotorSpeed       = mHingeMotorSpeed;
        def.MaxMotorTorque   = mHingeMaxMotorTorque;

        mHingeHandle = physicsService->CreateHinge(def);
        mConstraints.push_back(mHingeHandle);
    }

    // ── Spring chain ─────────────────────────────────────────────
    mChainBodyHandles.clear();
    if (mChainStartIdx >= 0) {
        // Collect chain body handles
        for (int i = 0; i < CHAIN_LINK_COUNT; ++i) {
            Umbra::BodyHandle h = getHandle(mChainStartIdx + i);
            if (!h.IsValid()) return; // not ready yet
            mChainBodyHandles.push_back(h);
        }

        // Create springs between consecutive links
        for (int i = 0; i < CHAIN_LINK_COUNT - 1; ++i) {
            Umbra::SpringDef def;
            def.BodyA      = mChainBodyHandles[i];
            def.BodyB      = mChainBodyHandles[i + 1];
            def.Stiffness  = mChainStiffness;
            def.Damping    = mChainDamping;
            def.RestLength = mChainRestLength;

            Umbra::ConstraintHandle ch = physicsService->CreateSpring(def);
            mConstraints.push_back(ch);
        }
    }
}

// ────────────────────────────────────────────────────────────────
// Cleanup
// ────────────────────────────────────────────────────────────────

void ConstraintJointScene::CleanupScene() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (auto& handle : mConstraints) {
        if (physicsService->IsConstraintValid(handle)) {
            physicsService->DestroyConstraint(handle);
        }
    }
    mConstraints.clear();

    mSpringHandle   = Umbra::ConstraintHandle::Invalid();
    mDistanceHandle = Umbra::ConstraintHandle::Invalid();
    mHingeHandle    = Umbra::ConstraintHandle::Invalid();

    for (auto entity : mEntities) {
        world->DestroyEntity(entity);
    }
    mEntities.clear();

    mChainBodyHandles.clear();
    mChainStartIdx        = -1;
    bNeedsConstraintSetup = false;
}

// ────────────────────────────────────────────────────────────────
// Debug drawing
// ────────────────────────────────────────────────────────────────

void ConstraintJointScene::DrawConstraintLines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // ── Spring line (yellow) ────────────────────────────────────
    if (mSpringBodyHandle.IsValid()) {
        Umbra::Math::Vector2f bodyPos = physicsService->GetPosition(mSpringBodyHandle);
        Umbra::RenderSystem::DebugDrawLine(mSpringWorldAnchor, bodyPos, Umbra::Color::Yellow);
        Umbra::RenderSystem::DebugDrawCircle(mSpringWorldAnchor, 4.0f, true, Umbra::Color::Yellow);
    }

    // ── Distance line (cyan) ────────────────────────────────────
    if (mDistanceBodyA.IsValid() && mDistanceBodyB.IsValid()) {
        Umbra::Math::Vector2f posA = physicsService->GetPosition(mDistanceBodyA);
        Umbra::Math::Vector2f posB = physicsService->GetPosition(mDistanceBodyB);
        Umbra::RenderSystem::DebugDrawLine(posA, posB, Umbra::Color::Cyan);
        Umbra::RenderSystem::DebugDrawCircle(posA, 4.0f, true, Umbra::Color::Cyan);
    }

    // ── Spring chain lines (green) ──────────────────────────────
    for (int i = 0; i + 1 < (int)mChainBodyHandles.size(); ++i) {
        if (mChainBodyHandles[i].IsValid() && mChainBodyHandles[i + 1].IsValid()) {
            Umbra::Math::Vector2f posA = physicsService->GetPosition(mChainBodyHandles[i]);
            Umbra::Math::Vector2f posB = physicsService->GetPosition(mChainBodyHandles[i + 1]);
            Umbra::RenderSystem::DebugDrawLine(posA, posB, Umbra::Color::Green);
        }
    }

    // ── Hinge marker (magenta) ──────────────────────────────────
    if (mHingeBodyA.IsValid()) {
        Umbra::Math::Vector2f posA = physicsService->GetPosition(mHingeBodyA);
        Umbra::RenderSystem::DebugDrawCircle(posA, 6.0f, true, Umbra::Color::Magenta);

        if (mHingeBodyB.IsValid()) {
            Umbra::Math::Vector2f posB = physicsService->GetPosition(mHingeBodyB);
            Umbra::RenderSystem::DebugDrawLine(posA, posB, Umbra::Color::Magenta);
        }
    }
}

void ConstraintJointScene::DrawColliderOutlines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (auto entity : mEntities) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(entity);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(entity);
        if (rb == nullptr || transform == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody == nullptr) {
            continue;
        }

        Umbra::Color color = (physBody->InverseMass == 0.0f) ? Umbra::Color(0.4f, 0.4f, 0.4f) : Umbra::Color::White;

        if (physBody->BodyShape.IsCircle()) {
            Umbra::RenderSystem::DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), false, color);
        } else if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, false, color);
        }
    }
}
