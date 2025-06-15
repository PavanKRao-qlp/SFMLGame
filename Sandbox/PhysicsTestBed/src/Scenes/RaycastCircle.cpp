#include "RaycastCircle.h"

#include "Core/Random.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Umbra.h"
#include "imgui.h"


void RaycastCircle::Initialize() {
    if (GetCameraEntity() != Umbra::MAX_ENTITY) {
        GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(200);
    }
}

void RaycastCircle::OnFixedUpdated() {
    mRay.Position                         = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    mRay.Direction                        = Umbra::Math::Vector2f(1, 0).GetRotated(mAngle);
    Umbra::Math::Vector2f rayOrigToCircle = mPoint - mRay.Position;
    float proj                            = Umbra::Math::Vector2f::Dot(rayOrigToCircle, mRay.Direction);
    Umbra::Math::Vector2f projPoint       = mRay.Position + (proj * mRay.Direction);
    float projHeight                      = (mPoint - projPoint).Magnitude();
    // x2 + y2 = r2
    // x = sqrt (r2 - y2)
    float distToEdge                 = Umbra::Math::Sqrt(mRadius * mRadius - projHeight * projHeight);
    bool bCastHit                    = (proj - distToEdge) > 0;
    Umbra::Math::Vector2f castPointA = projPoint + (mRay.Direction * distToEdge);
    Umbra::Math::Vector2f castPointB = projPoint - (mRay.Direction * distToEdge);
    Umbra::RenderSystem::DebugDrawCircle(mPoint, mRadius, false, !bCastHit ? sf::Color::Red : sf::Color::Green);
    Umbra::RenderSystem::DebugDrawCircle(mRay.Position, 2, true, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawCircle(projPoint, 2, true, sf::Color::Magenta);
    Umbra::RenderSystem::DebugDrawCircle(castPointA, 2, true, sf::Color::Yellow);
    Umbra::RenderSystem::DebugDrawCircle(castPointB, 2, true, sf::Color::Blue);
    Umbra::RenderSystem::DebugDrawLine(mRay.Position, mRay.Position + mRay.Direction * 500, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(mRay.Position, mPoint, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(mPoint, projPoint, sf::Color::Magenta);
}

void RaycastCircle::OnUpdate() {
    ImGui::Begin("Circle Raycast");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::SliderFloat("Angle", &mAngle, 0, 360);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> RaycastCircle::InsatiateCopy() {
    return std::make_shared<RaycastCircle>(*this);
}

void RaycastCircle::OnBeginPlay() {
    int transformX = Umbra::Random::RandomRange(-75, 75);
    int transformY = Umbra::Random::RandomRange(-75, 75);
    mPoint         = Umbra::Math::Vector2f(transformX, transformY);
    mRadius        = Umbra::Random::RandomRange(30, 70);
    mAngle         = Umbra::Random::RandomRange(0, 360);
}

void RaycastCircle::OnEndPlay() {}
