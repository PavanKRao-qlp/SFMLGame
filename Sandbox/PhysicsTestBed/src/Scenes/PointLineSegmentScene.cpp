#include "PointLineSegmentScene.h"

#include "Core/Random.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Umbra.h"
#include "imgui.h"

void PointLineSegmentScene::Initialize() {
    if (GetCameraEntity() != Umbra::MAX_ENTITY) {
        GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(100);
    }
}

void PointLineSegmentScene::OnFixedUpdated() {
    Umbra::RenderSystem::DebugDrawLine(mPointA, mPointB);
    Umbra::Math::Vector2f worldMousePos = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::Math::Vector2f onLinePos     = Umbra::Math::GetClosestPointInLineSegment(mPointA, mPointB, worldMousePos);
    Umbra::Math::Vector2f normal =
        Umbra::Math::GetOrthogonalNormalToPointForLineSegment(mPointA, mPointB, worldMousePos);
    mDistance      = Umbra::Math::GetDistanceToClosestPointInLineSegment(mPointA, mPointB, worldMousePos);
    mOrthoDistance = Umbra::Math::GetOrthogonalDistanceFromPointToLineSegment(mPointA, mPointB, worldMousePos);
    bOnLine        = Umbra::Math::IsPointOnLineSegment(mPointA, mPointB, worldMousePos, 1);
    Umbra::RenderSystem::DebugDrawCircle(worldMousePos, 1.5f, true, sf::Color::Red);
    Umbra::RenderSystem::DebugDrawCircle(onLinePos, 1.f, true, sf::Color::Green);
    Umbra::RenderSystem::DebugDrawLine(worldMousePos, worldMousePos - (worldMousePos - onLinePos), sf::Color::Red);
    Umbra::Math::Vector2f midPoint = mPointA + (mPointB - mPointA) * 0.5f;
    Umbra::RenderSystem::DebugDrawCircle(midPoint, 1.f, true, sf::Color::Blue);
    Umbra::RenderSystem::DebugDrawLine(midPoint, midPoint + (normal * 20), sf::Color::Blue);
}

void PointLineSegmentScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    ImGui::Begin("Point Line Segment");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::Text("Distance :: %f", mDistance);
    ImGui::Text("Orthogonal Distance :: %f", mOrthoDistance);
    ImGui::Checkbox("Is On the Line", &bOnLine);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> PointLineSegmentScene::InsatiateCopy() {
    return std::make_shared<PointLineSegmentScene>(*this);
}

void PointLineSegmentScene::OnBeginPlay() {
    float randomX = Umbra::Random::RandomRange(-75, 75);
    float randomY = Umbra::Random::RandomRange(-75, 75);
    mPointA       = Umbra::Math::Vector2f(randomX, randomY);

    randomX = Umbra::Random::RandomRange(-75, 75);
    randomY = Umbra::Random::RandomRange(-75, 100);
    mPointB = Umbra::Math::Vector2f(randomX, randomY);
}

void PointLineSegmentScene::OnEndPlay() {}
