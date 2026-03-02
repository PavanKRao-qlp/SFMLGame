#include "ParticleIntegrationScene.h"

#include "Core/Clock.h"
#include "Core/Random.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/LifeTimeSystem.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void ParticleIntegrationScene::Initialize() {
    CreateDefaultCamera(100.0f);

    Umbra::World* world = GetWorld();
    world->AddSystem(Umbra::ESystemPhase::Simulation, 5, std::make_shared<Umbra::LifeTimeSystem>());
}

void ParticleIntegrationScene::OnFixedUpdate() {}

void ParticleIntegrationScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }

    // Auto-spawn timer
    if (bAutoSpawn) {
        mSpawnTimer += Umbra::EngineTime::GetDeltaTime();
        while (mSpawnTimer >= mSpawnInterval) {
            mSpawnTimer -= mSpawnInterval;
            for (int i = 0; i < mParticlesPerSpawn; ++i) {
                SpawnParticle();
            }
        }
    }

    // UI
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);
    ImGui::Begin("Particle Object : Integration");

    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }

    ImGui::Separator();

    // Spawn controls
    if (ImGui::CollapsingHeader("Spawn Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Auto Spawn", &bAutoSpawn);
        ImGui::SliderFloat("Spawn Interval", &mSpawnInterval, 0.05f, 5.0f, "%.2f s");
        ImGui::SliderInt("Per Spawn", &mParticlesPerSpawn, 1, 20);
        ImGui::SliderFloat("Lifetime", &mParticleLifetime, 0.5f, 30.0f, "%.1f s");
        ImGui::Separator();
        ImGui::SliderFloat("Min Speed", &mMinSpeed, 0.0f, 100.0f);
        ImGui::SliderFloat("Max Speed", &mMaxSpeed, 0.0f, 100.0f);
        ImGui::SliderFloat(
            "Min Angular Speed", &mMinAngularSpeed, -Umbra::Math::PI * 2.0f * 5, Umbra::Math::PI * 2.0f * 5);
        ImGui::SliderFloat(
            "Max Angular Speed", &mMaxAngularSpeed, -Umbra::Math::PI * 2.0f * 5, Umbra::Math::PI * 2.0f * 5);
        ImGui::SliderFloat("Particle Size", &mParticleSize, 0.2f, 10.0f);
        ImGui::SliderFloat("Particle Mass", &mParticleMass, 0.1f, 100.0f);

        if (ImGui::Button("Spawn Now")) {
            for (int i = 0; i < mParticlesPerSpawn; ++i) {
                SpawnParticle();
            }
        }
        ImGui::Separator();
    }

    // Physics World Config
    DrawPhysicsWorldConfigWidget();

    // Stats
    Umbra::PhysicsService* physics = GetWorld()->GetPhysicsService();
    if (physics) {
        ImGui::Separator();
        ImGui::Text("Active Bodies: %u", physics->GetBodyCount());
    }

    ImGui::End();
}

void ParticleIntegrationScene::SpawnParticle() {
    Umbra::World* world = GetWorld();

    Umbra::EntityID entity = world->CreateEntity();

    // Random spawn position near origin
    float spawnX = Umbra::Random::RandomRange(-5.0f, 5.0f);
    float spawnY = Umbra::Random::RandomRange(-5.0f, 5.0f);

    world->AddComponent<Umbra::TransformComponent>(
        entity, Umbra::Math::Vector2f(spawnX, spawnY), Umbra::Math::Vector2f(mParticleSize, mParticleSize));

    // Random color
    Umbra::Color color(Umbra::Random::RandomRange(0.3f, 1.0f), Umbra::Random::RandomRange(0.3f, 1.0f),
        Umbra::Random::RandomRange(0.3f, 1.0f));
    world->AddComponent<Umbra::SpriteComponent>(entity, color);

    // Rigidbody via physics service
    Umbra::RigidbodyHandleComponent rb;
    rb.Mass               = mParticleMass;
    rb.bAffectedByGravity = true;

    // Random velocity direction
    float angle       = Umbra::Random::RandomRange(0.0f, Umbra::Math::PI * 2.0f);
    float speed       = Umbra::Random::RandomRange(mMinSpeed, mMaxSpeed);
    rb.CachedVelocity = Umbra::Math::Vector2f(Umbra::Math::Cos(angle) * speed, Umbra::Math::Sin(angle) * speed);

    rb.CachedAngularVelocity = Umbra::Random::RandomRange(mMinAngularSpeed, mMaxAngularSpeed);

    world->AddComponent<Umbra::RigidbodyHandleComponent>(entity, rb);

    // Auto-destroy after lifetime
    world->AddComponent<Umbra::LifeTimeComponent>(entity, mParticleLifetime);
}

void ParticleIntegrationScene::DrawPhysicsWorldConfigWidget() {
    Umbra::PhysicsService* physics = GetWorld()->GetPhysicsService();
    if (!physics) {
        return;
    }

    if (ImGui::CollapsingHeader("Physics World Config")) {
        // Gravity
        Umbra::Math::Vector2f gravity = physics->GetGravity();
        float gravityArr[2]           = {gravity.x, gravity.y};
        if (ImGui::DragFloat2("Gravity", gravityArr, 0.1f, -100.0f, 100.0f)) {
            physics->SetGravity(Umbra::Math::Vector2f(gravityArr[0], gravityArr[1]));
        }

        // Damping
        float linearDamping = physics->GetLinearDamping();
        if (ImGui::SliderFloat("Linear Damping", &linearDamping, 0.0f, 1.0f)) {
            physics->SetLinearDamping(linearDamping);
        }

        float angularDamping = physics->GetAngularDamping();
        if (ImGui::SliderFloat("Angular Damping", &angularDamping, 0.0f, 1.0f)) {
            physics->SetAngularDamping(angularDamping);
        }
    }
}

Umbra::SharedPtr<Umbra::Scene> ParticleIntegrationScene::InstantiateCopy() {
    return std::make_shared<ParticleIntegrationScene>(*this);
}

void ParticleIntegrationScene::OnBeginPlay() {}

void ParticleIntegrationScene::OnEndPlay() {}
