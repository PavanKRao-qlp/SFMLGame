#include "RaycastLineSegment.h"

#include "Core/Random.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "Service/ServiceLocator.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Graphics/Color.h"
#include "Umbra.h"
#include "imgui.h"

void RaycastLineSegment::Initialize() {
    CreateDefaultCamera(200.0f);
}

void RaycastLineSegment::OnFixedUpdate() {
    mRay.Position  = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    mRay.Direction = Umbra::Math::Vector2f(1, 0).GetRotated(mAngle);
    Umbra::Math::Vector2f pos;
    if (Umbra::Math::TestRayLineSegment(mRay, mPointA, mPointB, pos)) {
        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pos, 2, true, Umbra::Color::Green);
    }
    Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(mRay.Position, mRay.Position + mRay.Direction * 500, Umbra::Color::Cyan);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(mRay.Position, 2, true, Umbra::Color::Cyan);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(mPointA, 2, true, Umbra::Color::Red);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(mPointB, 2, true, Umbra::Color::Red);
    Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(mPointA, mPointB, Umbra::Color::Red);
}

void RaycastLineSegment::OnUpdate() {
    ImGui::Begin("Segment Raycast");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    // ImGui::Text("rect size x %f", transform->Size.x);
    // ImGui::Text("rect size y     %f", transform->Size.y);
    // ImGui::SliderFloat("Angle", &transform->Angle, 0, 360);
    // ImGui::SliderFloat("Radius", &mRadius, 10, 50);
    // ImGui::Text("distance     %f", mDistance);
    // ImGui::SliderFloat("Radius", &mRadius, 5, 50);
    ImGui::SliderFloat("Angle", &mAngle, 0, 360);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> RaycastLineSegment::InstantiateCopy() {
    return std::make_shared<RaycastLineSegment>(*this);
}

void RaycastLineSegment::OnBeginPlay() {
    int transformX = Umbra::Random::RandomRange(-75, 75);
    int transformY = Umbra::Random::RandomRange(-75, 75);
    mPointA        = Umbra::Math::Vector2f(transformX, transformY);
    transformX     = Umbra::Random::RandomRange(-75, 75);
    transformY     = Umbra::Random::RandomRange(-75, 75);
    mPointA        = Umbra::Math::Vector2f(transformX, transformY);
}

void RaycastLineSegment::OnEndPlay() {}
