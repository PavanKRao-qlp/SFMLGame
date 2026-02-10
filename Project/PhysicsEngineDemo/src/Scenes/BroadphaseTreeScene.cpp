#include "BroadphaseTreeScene.h"

#include "Core/Random.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Math/Bounds.h"
#include "Service/Physics/DynamicAABBTree.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void BroadphaseTreeScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void BroadphaseTreeScene::OnFixedUpdate() {}

void BroadphaseTreeScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    // --- ImGui Panel ---
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 340), ImGuiCond_FirstUseEver);
    ImGui::Begin("Broadphase AABB Tree");

    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();

    ImGui::Text("Visualization Toggles");
    ImGui::Checkbox("Colliders", &bShowColliders);
    ImGui::Checkbox("Tight AABBs (Red)", &bShowTightAABBs);
    ImGui::Checkbox("Fat AABBs (Yellow)", &bShowFatAABBs);
    ImGui::Checkbox("Tree Nodes (Cyan)", &bShowTreeNodes);
    ImGui::Separator();

    auto bodyCount        = physicsService->GetBodyCount();
    auto treeHeight       = physicsService->GetBroadphaseTree().GetHeight();
    auto proxyCount       = physicsService->GetBroadphaseTree().GetProxyCount();
    auto bpChecks         = physicsService->GetBroadphaseChecks();
    auto bruteForce       = bodyCount * (bodyCount > 0 ? bodyCount - 1 : 0) / 2;
    auto pairsFound       = physicsService->GetBroadphasePairCount();
    auto narrowCollisions = static_cast<unsigned int>(physicsService->GetCollisions().size());

    ImGui::Text("Stats");
    ImGui::Text("Bodies: %u", bodyCount);
    ImGui::Text("Tree Height: %u", treeHeight);
    ImGui::Text("Proxies: %u", proxyCount);
    ImGui::Separator();
    ImGui::Text("Broadphase checks (tree): %u", bpChecks);
    ImGui::Text("Brute-force equivalent: %u", bruteForce);
    ImGui::Text("Pairs found: %u", pairsFound);
    ImGui::Text("Narrow phase collisions: %u", narrowCollisions);
    ImGui::Separator();

    if (ImGui::Button("Spawn More")) {
        SpawnRandomBody();
    }

    ImGui::End();

    // --- Drawing ---
    MovePlayer();
    if (bShowColliders) {
        DrawColliderOutlines();
    }
    if (bShowTightAABBs) {
        DrawTightAABBs();
    }
    if (bShowFatAABBs) {
        DrawFatAABBs();
    }
    if (bShowTreeNodes) {
        DrawTreeInternalNodes();
    }
}

Umbra::SharedPtr<Umbra::Scene> BroadphaseTreeScene::InstantiateCopy() {
    return std::make_shared<BroadphaseTreeScene>(*this);
}

void BroadphaseTreeScene::OnBeginPlay() {
    Umbra::World* world = GetWorld();
    mEntities.clear();

    // --- Static floor ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(0.0f, -180.0f), Umbra::Math::Vector2f(400.0f, 20.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.3f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(400.0f, 20.0f));
    }

    // --- Left wall ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(-200.0f, 0.0f), Umbra::Math::Vector2f(20.0f, 380.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.3f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(20.0f, 380.0f));
    }

    // --- Right wall ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(200.0f, 0.0f), Umbra::Math::Vector2f(20.0f, 380.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.3f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(20.0f, 380.0f));
    }

    // --- Ceiling ---
    {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(0.0f, 190.0f), Umbra::Math::Vector2f(400.0f, 20.0f), 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 0.0f;
        rb.bAffectedByGravity = false;
        rb.CoefOfRestitution  = 0.3f;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);
        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(400.0f, 20.0f));
    }

    // --- WASD kinematic player ---
    {
        mPlayerEntity = world->CreateEntity();
        mEntities.push_back(mPlayerEntity);

        Umbra::Math::Vector2f size(30.0f, 30.0f);
        world->AddComponent<Umbra::TransformComponent>(
            mPlayerEntity, Umbra::Math::Vector2f(0.0f, 0.0f), size, 0.0f);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = false;
        rb.bIsKinematic       = true;
        world->AddComponent<Umbra::RigidbodyHandleComponent>(mPlayerEntity, rb);
        world->AddComponent<Umbra::CircleColliderComponent>(mPlayerEntity, size.x * 0.5f);
    }

    // --- Dynamic circles ---
    for (int i = 0; i < 8; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        float posX   = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float posY   = Umbra::Random::RandomRange(-100.0f, 150.0f);
        float radius = Umbra::Random::RandomRange(10.0f, 22.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.5f, 0.9f);
        rb.CachedVelocity     = Umbra::Math::Vector2f(
            Umbra::Random::RandomRange(-30.0f, 30.0f), Umbra::Random::RandomRange(-15.0f, 15.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
    }

    // --- Dynamic boxes ---
    for (int i = 0; i < 7; ++i) {
        Umbra::EntityID entity = world->CreateEntity();
        mEntities.push_back(entity);

        float posX  = Umbra::Random::RandomRange(-150.0f, 150.0f);
        float posY  = Umbra::Random::RandomRange(-100.0f, 150.0f);
        float sizeX = Umbra::Random::RandomRange(18.0f, 40.0f);
        float sizeY = Umbra::Random::RandomRange(18.0f, 40.0f);
        float angle = Umbra::Random::RandomRange(-30.0f, 30.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(sizeX, sizeY), angle);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.5f, 0.9f);
        rb.CachedVelocity     = Umbra::Math::Vector2f(
            Umbra::Random::RandomRange(-25.0f, 25.0f), Umbra::Random::RandomRange(-10.0f, 10.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));
    }
}

void BroadphaseTreeScene::OnEndPlay() {
    mEntities.clear();
}

void BroadphaseTreeScene::MovePlayer() {
    Umbra::World* world                  = GetWorld();
    Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mPlayerEntity);
    if (transform == nullptr) {
        return;
    }

    float dt = Umbra::GEngineStatics.GameConfig->FixedDeltaTime;
    Umbra::Math::Vector2f direction(0.0f, 0.0f);
    if (Umbra::Input::GetKey(Umbra::KeyBoard::W)) {
        direction.y += 1.0f;
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::S)) {
        direction.y -= 1.0f;
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::A)) {
        direction.x -= 1.0f;
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::D)) {
        direction.x += 1.0f;
    }
    transform->Position += direction * mPlayerSpeed * dt;
}

void BroadphaseTreeScene::SpawnRandomBody() {
    Umbra::World* world = GetWorld();
    Umbra::EntityID entity = world->CreateEntity();
    mEntities.push_back(entity);

    float posX = Umbra::Random::RandomRange(-140.0f, 140.0f);
    float posY = Umbra::Random::RandomRange(50.0f, 160.0f);

    bool isCircle = Umbra::Random::RandomRange(0.0f, 1.0f) > 0.5f;
    if (isCircle) {
        float radius = Umbra::Random::RandomRange(10.0f, 22.0f);
        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(radius * 2, radius * 2));

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.5f, 0.9f);
        rb.CachedVelocity     = Umbra::Math::Vector2f(
            Umbra::Random::RandomRange(-20.0f, 20.0f), Umbra::Random::RandomRange(-10.0f, 10.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        Umbra::CircleColliderComponent circle;
        circle.Radius = radius;
        world->AddComponent<Umbra::CircleColliderComponent>(entity, circle);
    } else {
        float sizeX = Umbra::Random::RandomRange(18.0f, 40.0f);
        float sizeY = Umbra::Random::RandomRange(18.0f, 40.0f);
        float angle = Umbra::Random::RandomRange(-30.0f, 30.0f);

        world->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::Math::Vector2f(posX, posY), Umbra::Math::Vector2f(sizeX, sizeY), angle);

        Umbra::RigidbodyHandleComponent rb;
        rb.Mass               = 1.0f;
        rb.bAffectedByGravity = true;
        rb.CoefOfRestitution  = Umbra::Random::RandomRange(0.5f, 0.9f);
        rb.CachedVelocity     = Umbra::Math::Vector2f(
            Umbra::Random::RandomRange(-20.0f, 20.0f), Umbra::Random::RandomRange(-10.0f, 10.0f));
        world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

        world->AddComponent<Umbra::BoxColliderComponent>(entity, Umbra::Math::Vector2f(sizeX, sizeY));
    }
}

void BroadphaseTreeScene::DrawColliderOutlines() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (rb == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody     = physicsService->GetBodyData(rb->Handle);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (physBody == nullptr || transform == nullptr) {
            continue;
        }

        Umbra::Color color = physBody->IsStatic() ? Umbra::Color(100, 100, 100) : Umbra::Color::White;
        if (mEntities[i] == mPlayerEntity) {
            color = Umbra::Color::Green;
        }

        if (physBody->BodyShape.IsCircle()) {
            Umbra::RenderSystem::DebugDrawCircle(
                transform->Position, physBody->BodyShape.GetCircle().GetRadius(), false, color);
        } else if (physBody->BodyShape.IsBox()) {
            Umbra::Math::Bounds2D bounds(transform->Position, physBody->BodyShape.GetBox().GetSize());
            Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, false, color);
        }
    }
}

void BroadphaseTreeScene::DrawTightAABBs() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (rb == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody     = physicsService->GetBodyData(rb->Handle);
        Umbra::TransformComponent* transform = world->GetComponent<Umbra::TransformComponent>(mEntities[i]);
        if (physBody == nullptr || transform == nullptr) {
            continue;
        }

        Umbra::Math::Bounds2D bounds(transform->Position, physBody->BoundingAABB.Size);
        Umbra::RenderSystem::DrawDebugBox(bounds, false, Umbra::Color::Red);
    }
}

void BroadphaseTreeScene::DrawFatAABBs() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();
    const Umbra::DynamicAABBTree& tree    = physicsService->GetBroadphaseTree();

    for (size_t i = 0; i < mEntities.size(); ++i) {
        Umbra::RigidbodyHandleComponent* rb = world->GetComponent<Umbra::RigidbodyHandleComponent>(mEntities[i]);
        if (rb == nullptr || !rb->Handle.IsValid()) {
            continue;
        }

        Umbra::PhysicsBodyData* physBody = physicsService->GetBodyData(rb->Handle);
        if (physBody == nullptr || physBody->TreeProxyId == Umbra::NullNode) {
            continue;
        }

        const Umbra::Math::Bounds2D& fatAabb = tree.GetFatAABB(physBody->TreeProxyId);
        Umbra::RenderSystem::DrawDebugBox(fatAabb, false, Umbra::Color::Yellow);
    }
}

void BroadphaseTreeScene::DrawTreeInternalNodes() {
    Umbra::World* world                   = GetWorld();
    Umbra::PhysicsService* physicsService = world->GetPhysicsService();
    const Umbra::DynamicAABBTree& tree    = physicsService->GetBroadphaseTree();

    Umbra::int32 rootId = tree.GetRootNodeId();
    if (rootId == Umbra::NullNode) {
        return;
    }

    unsigned int treeHeight = tree.GetHeight();
    if (treeHeight == 0) {
        return;
    }

    // Traverse the tree iteratively, drawing internal (non-leaf) nodes
    struct TraversalEntry {
        Umbra::int32 NodeId;
        unsigned int Depth;
    };

    Umbra::Stack<TraversalEntry> stack;
    TraversalEntry rootEntry;
    rootEntry.NodeId = rootId;
    rootEntry.Depth  = 0;
    stack.push(rootEntry);

    while (!stack.empty()) {
        TraversalEntry entry = stack.top();
        stack.pop();

        if (entry.NodeId == Umbra::NullNode) {
            continue;
        }

        const Umbra::AABBTreeNode& node = tree.GetNode(entry.NodeId);

        // Only draw internal nodes (non-leaf)
        if (!node.IsLeaf()) {
            // Depth-fading alpha: deeper nodes are more transparent
            float alphaFraction = 1.0f - static_cast<float>(entry.Depth) / static_cast<float>(treeHeight + 1);
            uint8_t alpha       = static_cast<uint8_t>(alphaFraction * 180.0f + 40.0f);
            Umbra::Color color  = Umbra::Color::FromU8(0, 255, 255, alpha);

            Umbra::RenderSystem::DrawDebugBox(node.Aabb, false, color);

            TraversalEntry leftEntry;
            leftEntry.NodeId = node.Left;
            leftEntry.Depth  = entry.Depth + 1;
            stack.push(leftEntry);

            TraversalEntry rightEntry;
            rightEntry.NodeId = node.Right;
            rightEntry.Depth  = entry.Depth + 1;
            stack.push(rightEntry);
        }
    }
}
