#include "CircleLineSegmentScene.h"

#include "Core/Random.h"
#include "Graphics/Color.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Math/MathUtils.h"
#include "PointLineSegmentScene.h"
#include "Umbra.h"
#include "imgui.h"

void CircleLineSegmentScene::Initialize() {
    CreateDefaultCamera(100.0f);
}

void CircleLineSegmentScene::OnFixedUpdate() {
    Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(mPointA, mPointB);
    Umbra::Math::Vector2f worldMousePos = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::Math::Vector2f onLinePos     = Umbra::Math::GetClosestPointInLineSegment(mPointA, mPointB, worldMousePos);
    Umbra::Math::Vector2f normal =
        Umbra::Math::GetOrthogonalNormalToPointForLineSegment(mPointA, mPointB, worldMousePos);
    mDistance = Umbra::Math::GetDistanceToClosestPointInLineSegment(mPointA, mPointB, worldMousePos) - mRadius;
    mOrthoDistance =
        Umbra::Math::GetOrthogonalDistanceFromPointToLineSegment(mPointA, mPointB, worldMousePos) - mRadius;
    bool bColliding = (mDistance <= 0);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(worldMousePos, mRadius, false, bColliding ? Umbra::Color::Green : Umbra::Color::Red);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(onLinePos, 1.f, true, Umbra::Color::Green);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(worldMousePos, worldMousePos - (worldMousePos - onLinePos), Umbra::Color::Red);
    Umbra::Math::Vector2f midPoint = mPointA + (mPointB - mPointA) * 0.5f;
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(midPoint, 1.f, true, Umbra::Color::Blue);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(midPoint, midPoint + (normal * 20), Umbra::Color::Blue);
}

void CircleLineSegmentScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    ImGui::Begin("Circle Line Segment");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::Text("Distance :: %f", mDistance);
    ImGui::Text("Orthogonal Distance :: %f", mOrthoDistance);
    ImGui::SliderFloat("Radius", &mRadius, 5, 50);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> CircleLineSegmentScene::InstantiateCopy() {
    return std::make_shared<CircleLineSegmentScene>(*this);
}

void CircleLineSegmentScene::OnBeginPlay() {
    float randomX = Umbra::Random::RandomRange(-75, 75);
    float randomY = Umbra::Random::RandomRange(-75, 75);
    mPointA       = Umbra::Math::Vector2f(randomX, randomY);

    randomX = Umbra::Random::RandomRange(-75, 75);
    randomY = Umbra::Random::RandomRange(-75, 100);
    mPointB = Umbra::Math::Vector2f(randomX, randomY);
}

void CircleLineSegmentScene::OnEndPlay() {}
