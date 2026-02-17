#include "CircleAABBScene.h"

#include "Core/Random.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Umbra.h"
#include "Graphics/Color.h"
#include "imgui.h"

void CircleAABBScene::Initialize() {
    CreateDefaultCamera(100.0f);
}

void CircleAABBScene::OnFixedUpdate() {
    Umbra::Math::Vector2f worldMousePos = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::Math::Bounds2D RectAABB(mPoint, mSize);
    Umbra::Math::Vector2f pointWithinBox = Umbra::Math::GetClosestPointInsideBound(RectAABB, worldMousePos);
    Umbra::Math::Vector2f pointOnBox     = Umbra::Math::GetClosestPointOnBoundEdge(RectAABB, worldMousePos);
    mDistance                            = (worldMousePos - pointWithinBox).Magnitude();
    mDistanceToProj                      = (worldMousePos - pointOnBox).Magnitude();
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(worldMousePos, 1.5f, true, Umbra::Color::Red);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(mPoint, 1.f, true, Umbra::Color::White);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pointOnBox, 1.2f, true, Umbra::Color::Yellow);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pointWithinBox, 1.f, true, Umbra::Color::Green);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawBox(RectAABB);
    bool bColliding = (mDistance <= mRadius);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(worldMousePos, mRadius, false, bColliding ? Umbra::Color::Green : Umbra::Color::Red);
}

void CircleAABBScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    ImGui::Begin("Circle AABB overlap distance");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::Text("Distance :: %f", mDistance);
    ImGui::Text("Distance to edge :: %f", mDistanceToProj);
    ImGui::SliderFloat("Radius", &mRadius, 5, 50);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> CircleAABBScene::InstantiateCopy() {
    return std::make_shared<CircleAABBScene>(*this);
}

void CircleAABBScene::OnBeginPlay() {

    float randomX = Umbra::Random::RandomRange(-75, 75);
    float randomY = Umbra::Random::RandomRange(-75, 75);
    mPoint        = Umbra::Math::Vector2f(randomX, randomY);

    randomX = Umbra::Random::RandomRange(0, 90);
    randomY = Umbra::Random::RandomRange(0, 90);
    mSize   = Umbra::Math::Vector2f(randomX, randomY);
}

void CircleAABBScene::OnEndPlay() {}
