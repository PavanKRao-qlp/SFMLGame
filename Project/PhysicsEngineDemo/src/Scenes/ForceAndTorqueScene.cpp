#include "ForceAndTorqueScene.h"

#include "Core/Clock.h"
#include "Core/Random.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Components/RigidbodyHandle.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "Service/Physics/PhysicsService.h"
#include "Umbra.h"
#include "imgui.h"

void ForceAndTorqueScene::Initialize() {
    CreateDefaultCamera(250.0f);
}

void ForceAndTorqueScene::OnFixedUpdate() {}

void ForceAndTorqueScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetSceneManager().GoToScene("MainScene");
    }
    if (mCachedEntity != Umbra::MAX_ENTITY) {
        if (Umbra::Input::GetKey(Umbra::KeyBoard::A)) {
            GetWorld()->ApplyForce(mCachedEntity, Umbra::Math::Vector2f(-mLinearForce, 0));
        }
        if (Umbra::Input::GetKey(Umbra::KeyBoard::D)) {
            GetWorld()->ApplyForce(mCachedEntity, Umbra::Math::Vector2f(mLinearForce, 0));
        }
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Space)) {
            GetWorld()->ApplyImpulse(mCachedEntity, Umbra::Math::Vector2f(0, mImpulseForce));
        }
        if (Umbra::Input::GetKey(Umbra::KeyBoard::E)) {
            GetWorld()->ApplyTorque(mCachedEntity, -mTorque);
        }
        if (Umbra::Input::GetKey(Umbra::KeyBoard::Q)) {
            GetWorld()->ApplyTorque(mCachedEntity, mTorque);
        }
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::C)) {
            GetWorld()->ApplyAngularImpulse(mCachedEntity, -mAngularImpulse);
        }
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Z)) {
            GetWorld()->ApplyAngularImpulse(mCachedEntity, mAngularImpulse);
        }
        if (Umbra::Input::GetMouseButtonDown(Umbra::Mouse::Left)) {
            Umbra::Math::Vector2f mouseToWorldPosition =
                GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
            GetWorld()->ApplyForceAtPoint(mCachedEntity, mForceVector, mouseToWorldPosition);
        }
    }
    // UI
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);
    ImGui::Begin("Particle Object : Force");

    if (ImGui::Button("Back to Main Menu")) {
        GetSceneManager().GoToScene("MainScene");
    }
    ImGui::Separator();
    ImGui::SliderFloat("Particle Mass", &mParticleMass, 0.0f, 100.0f);
    ImGui::SliderFloat("Particle Inertia", &mParticleInertia, 0.0f, 100.0f);
    ImGui::SliderFloat("Linear Force", &mLinearForce, 0.1f, 100.0f);
    ImGui::Text("Press A to add force to left");
    ImGui::Text("Press D to add force to right");
    ImGui::SliderFloat("Impulse Force", &mImpulseForce, 0.1f, 500.0f);
    ImGui::Text("Press space to add impulse to up");
    ImGui::Text("Press Q to add torque to left");
    ImGui::Text("Press E to add torque to right");
    ImGui::SliderFloat("Toque", &mTorque, 0.1f, 100.0f);
    ImGui::SliderFloat("Angular Impulse", &mAngularImpulse, 0.1f, 100.0f);
    ImGui::End();

    if (mCachedEntity != Umbra::MAX_ENTITY) {
        Umbra::RigidbodyHandleComponent* entityRigidbody =
            GetWorld()->GetComponent<Umbra::RigidbodyHandleComponent>(mCachedEntity);
        entityRigidbody->Mass    = mParticleMass;
        entityRigidbody->Inertia = mParticleInertia;
    }
}

Umbra::SharedPtr<Umbra::Scene> ForceAndTorqueScene::InstantiateCopy() {
    return std::make_shared<ForceAndTorqueScene>(*this);
}

void ForceAndTorqueScene::OnBeginPlay() {
    Umbra::World* world = GetWorld();

    mCachedEntity = world->CreateEntity();

    // Random spawn position near origin
    float spawnX = Umbra::Random::RandomRange(-5.0f, 5.0f);
    float spawnY = Umbra::Random::RandomRange(-5.0f, 5.0f);

    world->AddComponent<Umbra::TransformComponent>(
        mCachedEntity, Umbra::Math::Vector2f(spawnX, spawnY), Umbra::Math::Vector2f(30, 30));

    // Random color
    Umbra::Color color(Umbra::Random::RandomRange(0.3f, 1.0f), Umbra::Random::RandomRange(0.3f, 1.0f),
        Umbra::Random::RandomRange(0.3f, 1.0f));
    world->AddComponent<Umbra::SpriteComponent>(mCachedEntity, color);

    // Rigidbody via physics service
    Umbra::RigidbodyHandleComponent rb;
    rb.Mass               = mParticleSize;
    rb.bAffectedByGravity = false;

    // Random velocity direction
    float angle = Umbra::Random::RandomRange(-Umbra::Math::PI * 2.0f, Umbra::Math::PI * 2.0f);

    // rb.CachedAngularVelocity = Umbra::Random::RandomRange(mMinAngularSpeed, mMaxAngularSpeed);

    world->AddComponent<Umbra::RigidbodyHandleComponent>(mCachedEntity, rb);
}

void ForceAndTorqueScene::OnEndPlay() {}
