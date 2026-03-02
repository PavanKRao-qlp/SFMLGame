#include "TrianglePointScene.h"

#include "Core/Random.h"
#include "Graphics/Color.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Math/Triangle.h"
#include "Umbra.h"
#include "imgui.h"


void TrianglePointScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void TrianglePointScene::OnFixedUpdate() {
    Umbra::Math::Vector2f worldMousePos = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::RenderSystem::DebugDrawCircle(worldMousePos, 1.5f, true, Umbra::Color::Cyan);
    auto vertices = mTriangle.GetVertices();
    auto normals  = mTriangle.GetNormals();
    for (int i = 0; i < vertices.size(); i++) {
        Umbra::Color color = Umbra::Color::Red;
        if (i == 0) {
            color = Umbra::Color::Green;
        } else if (i == 1) {
            color = Umbra::Color::Blue;
        } else if (i == 2) {
            color = Umbra::Color::Red;
        }
        Umbra::RenderSystem::DebugDrawLine(vertices[i], vertices[(i + 1) % vertices.size()], Umbra::Color::White);
        Umbra::RenderSystem::DebugDrawLine(normals[i] * -100, normals[i] * 100, color);
        Umbra::RenderSystem::DebugDrawCircle(vertices[i], 1.75f, true, color);
    }
    for (int i = 0; i < normals.size(); i++) {
        for (int j = 0; j < vertices.size(); j++) {
            float projection = Umbra::Math::Vector2f::Dot(vertices[j], normals[i]);
            // s = AP.AB/AB.AB since AB is normalized it will be 1
            Umbra::Math::Vector2f projectionOnNormalScaled = normals[i] * projection;
            Umbra::Color color                                = Umbra::Color::Red;
            if (j == 0) {
                color = Umbra::Color::Green;
            } else if (j == 1) {
                color = Umbra::Color::Blue;
            } else if (j == 2) {
                color = Umbra::Color::Red;
            }
            Umbra::RenderSystem::DebugDrawCircle(projectionOnNormalScaled, 1.75f, true, color);
            Umbra::RenderSystem::DebugDrawLine(vertices[j], projectionOnNormalScaled, color);
        }
    }
}

void TrianglePointScene::OnUpdate() {
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
            orthographic += 40 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    if (Umbra::Input::GetKeyDown(Umbra::KeyBoard::E)) {
        if (GetCameraEntity() != Umbra::MAX_ENTITY) {
            float orthographic =
                GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->GetOrthographicSize();
            orthographic -= 40 * Umbra::EngineTime::GetDeltaTime();
            GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(orthographic);
        }
    }
    ImGui::Begin("Triangle point distance");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> TrianglePointScene::InstantiateCopy() {
    return std::make_shared<TrianglePointScene>(*this);
}

void TrianglePointScene::OnBeginPlay() {
    Umbra::Math::Vector2f vectorA =
        Umbra::Math::Vector2f(Umbra::Random::RandomRange(-75, 75), Umbra::Random::RandomRange(-75, 75));
    Umbra::Math::Vector2f vectorB =
        Umbra::Math::Vector2f(Umbra::Random::RandomRange(-75, 75), Umbra::Random::RandomRange(-75, 75));
    Umbra::Math::Vector2f vectorC =
        Umbra::Math::Vector2f(Umbra::Random::RandomRange(-75, 75), Umbra::Random::RandomRange(-75, 75));

    mTriangle = Umbra::Math::Triangle(vectorA, vectorB, vectorC);
}

void TrianglePointScene::OnEndPlay() {}
