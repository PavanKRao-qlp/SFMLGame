#include "SimpleScene.h"

#include "ECS/Components/Collider.h"
#include "Graphics/Color.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "Physics/ForceGenerator.h"
#include "PointObjectSpawnSystem.h"
#include "Umbra.h"
#include "imgui.h"

void SimpleScene::Initialize() {
    UMBRA_LOG_INFO("SimpleScene Initialized !!");
    CreateDefaultCamera(150.0f);
    ObjectSpawner = std::make_shared<PointObjectSpawnSystem>();
    // GetWorld()->AddSystem(Umbra::ESystemPhase::FrameStart, 1, ObjectSpawner);
    // UMBRA_LOG_CRITICAL("%s", IMGUI_VERSION);
}

void SimpleScene::OnFixedUpdate() {}

void SimpleScene::OnUpdate() {
    bool bShow = true;
    ImGui::ShowDemoWindow(&bShow);
    ImGui::Begin("a", &bShow);
    ImGui::End();
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Space)) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Right)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x +=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Left)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.x -=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Up)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y +=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Down)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            GetWorld()->GetComponent<Umbra::TransformComponent>(GetCameraEntity())->Position.y -=
                10 * Umbra::EngineTime::GetDeltaTime();
        }
    }

    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::Q)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            float orthographic =
                GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic += 10 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::E)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            float orthographic =
                GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic -= 10 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    if (entity) {
        Umbra::Math::Vector2f movePos(0, 0);
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::W)) {
            movePos.y += 10 * Umbra::EngineTime::GetDeltaTime();
        }
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::S)) {
            movePos.y -= 10 * Umbra::EngineTime::GetDeltaTime();
        }
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::A)) {
            movePos.x -= 10 * Umbra::EngineTime::GetDeltaTime();
        }
        if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::D)) {
            movePos.x += 10 * Umbra::EngineTime::GetDeltaTime();
        }
        GetWorld()->GetComponent<Umbra::TransformComponent>(entity)->Position += movePos;
    }
}

Umbra::SharedPtr<Umbra::Scene> SimpleScene::InstantiateCopy() {
    return std::make_shared<SimpleScene>(*this);
}

void SimpleScene::OnBeginPlay() {
    // {
    //     entity = GetWorld()->CreateEntity();
    //     // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, Umbra::Color::Red);
    //     GetWorld()->AddComponent<Umbra::TransformComponent>(
    //         entity, Umbra::TransformComponent(Umbra::Math::Vector2f(0, -100), Umbra::Math::Vector2f(50, 50)));
    //     Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
    //     physicsBodyComponent.SetMass(1);
    //     physicsBodyComponent.bAffectedByGravity = false;
    //     physicsBodyComponent.mVelocity          = Umbra::Math::Vector2f(0, 0);
    //     GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity, physicsBodyComponent);
    //     Umbra::CircleColliderComponent circleColliderComponent;
    //     circleColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
    //     circleColliderComponent.Radius = 20;
    //     GetWorld()->AddComponent<Umbra::CircleColliderComponent>(entity, circleColliderComponent);
    // }
    {
        entity = GetWorld()->CreateEntity();
        GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, Umbra::Color::Red);
        GetWorld()->AddComponent<Umbra::TransformComponent>(
            entity, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 0), Umbra::Math::Vector2f(50, 50)));
        Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
        physicsBodyComponent.SetMass(1);
        physicsBodyComponent.bAffectedByGravity = false;
        physicsBodyComponent.mVelocity          = Umbra::Math::Vector2f(0, 0);
        GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity, physicsBodyComponent);
        Umbra::BoxColliderComponent boxColliderComponent;
        boxColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
        boxColliderComponent.Size   = Umbra::Math::Vector2f(50, 50);
        GetWorld()->AddComponent<Umbra::BoxColliderComponent>(entity, boxColliderComponent);
    }
    // {
    //     Umbra::EntityID entity2 = GetWorld()->CreateEntity();
    //     // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity2, sf::Color::Blue);
    //     GetWorld()->AddComponent<Umbra::TransformComponent>(
    //         entity2, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 40), Umbra::Math::Vector2f(20, 20)));
    //     Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
    //     physicsBodyComponent.SetMass(10);
    //     physicsBodyComponent.bAffectedByGravity = false;
    //     physicsBodyComponent.mVelocity          = Umbra::Math::Vector2f(0, -50);
    //     GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity2, physicsBodyComponent);

    //     Umbra::CircleColliderComponent circleColliderComponent;
    //     circleColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
    //     circleColliderComponent.Radius = 10;
    //     GetWorld()->AddComponent<Umbra::CircleColliderComponent>(entity2, circleColliderComponent);
    // }

    // {
    //     Umbra::EntityID entity2 = GetWorld()->CreateEntity();
    //     // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity2, sf::Color::Blue);
    //     GetWorld()->AddComponent<Umbra::TransformComponent>(
    //         entity2, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 100), Umbra::Math::Vector2f(20, 20)));
    //     Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
    //     physicsBodyComponent.SetMass(0);
    //     physicsBodyComponent.bAffectedByGravity = false;
    //     physicsBodyComponent.mVelocity          = Umbra::Math::Vector2f(0, 0);
    //     GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity2, physicsBodyComponent);

    //     Umbra::CircleColliderComponent circleColliderComponent;
    //     circleColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
    //     circleColliderComponent.Radius = 20;
    //     GetWorld()->AddComponent<Umbra::CircleColliderComponent>(entity2, circleColliderComponent);
    // }
    {
        Umbra::EntityID entity2 = GetWorld()->CreateEntity();
        // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity2, sf::Color::Blue);
        GetWorld()->AddComponent<Umbra::TransformComponent>(
            entity2, Umbra::TransformComponent(Umbra::Math::Vector2f(0, 100), Umbra::Math::Vector2f(20, 20)));
        Umbra::PhysicsBodyComponent physicsBodyComponent = Umbra::PhysicsBodyComponent();
        physicsBodyComponent.SetMass(0);
        physicsBodyComponent.bAffectedByGravity = false;
        physicsBodyComponent.mVelocity          = Umbra::Math::Vector2f(0, 0);
        GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity2, physicsBodyComponent);

        Umbra::BoxColliderComponent boxColliderComponent;
        boxColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
        boxColliderComponent.Size   = Umbra::Math::Vector2f(50, 50);
        GetWorld()->AddComponent<Umbra::BoxColliderComponent>(entity2, boxColliderComponent);
    }
    // // {

    // //     Umbra::EntityID entity3 = GetWorld()->CreateEntity();
    // //     GetWorld()->AddComponent<Umbra::SpriteComponent>(entity3, sf::Color::Green);
    // //     GetWorld()->AddComponent<Umbra::TransformComponent>(entity3, Umbra::TransformComponent(
    // //                                                                      Umbra::Math::Vector2f(30, 0),
    // //                                                                      Umbra::Math::Vector2f(20, 20)));
    // //     physicsBodyComponent = Umbra::PhysicsBodyComponent();
    // //     physicsBodyComponent.SetMass(0);
    // //     physicsBodyComponent.bAffectedByGravity = false;
    // //     physicsBodyComponent.mVelocity = Umbra::Math::Vector2f(0, 0);
    // //     GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity2, physicsBodyComponent);

    // //     circleColliderComponent.Offset = Umbra::Math::Vector2f(0, 0);
    // //     circleColliderComponent.Radius = 10;
    // //     GetWorld()->AddComponent<Umbra::CircleColliderComponent>(entity2, circleColliderComponent);
    // // }
    // // Umbra::SharedPtr<Umbra::IForceGenerator> spring = std::make_shared<Umbra::SpringForceGenerator>(entity,
    // entity2, 1.f, 70.f);
    // // GetWorld()->GetPhysicsSystem()->AddForceGenerator(spring);

    // // Umbra::EntityID entity3 = GetWorld()->CreateEntity();
    // // GetWorld()->AddComponent<Umbra::SpriteComponent>(entity3, sf::Color::Green);
    // // GetWorld()->AddComponent<Umbra::TransformComponent>(entity3, Umbra::TransformComponent(
    // //                                                                  Umbra::Math::Vector2f(0, 0),
    // //                                                                  Umbra::Math::Vector2f(45, 45)));
    // // physicsBodyComponent = Umbra::PhysicsBodyComponent();
    // // physicsBodyComponent.SetMass(0);
    // // physicsBodyComponent.mVelocity = Umbra::Math::Vector2f(0, 0);
    // // physicsBodyComponent.mAngularVelocity = Umbra::Math::PI * 2.f;
    // // GetWorld()->AddComponent<Umbra::PhysicsBodyComponent>(entity3, physicsBodyComponent);

    // //   entity = GetWorld()->CreateEntity();
    // //   GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Blue);
    // //   GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
    // //                                                                   Umbra::Math::Vector2f(0, 50),
    // //                                                                   Umbra::Math::Vector2f(50, 50)));
    // //   entity = GetWorld()->CreateEntity();
    // //   GetWorld()->AddComponent<Umbra::SpriteComponent>(entity, sf::Color::Green);
    // //   GetWorld()->AddComponent<Umbra::TransformComponent>(entity, Umbra::TransformComponent(
    // //                                                                   Umbra::Math::Vector2f(0, -50),
    // //                                                                   Umbra::Math::Vector2f(50, 50)));
    // //   GetWorld()->Create
    // //   //   GetWorld()->Create
}

void SimpleScene::OnEndPlay() {}
