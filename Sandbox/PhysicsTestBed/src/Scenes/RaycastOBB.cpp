#include "RaycastOBB.h"

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

void RaycastOBB::Initialize() {}

void RaycastOBB::OnFixedUpdated() {
    mRay.Position                        = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    mRay.Direction                       = Umbra::Math::Vector2f(1, 0).GetRotated(mAngle);
    Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mBoxEntity);
    Umbra::Math::Bounds2D bounds         = Umbra::Math::Bounds2D(transform->Position, transform->Size);
    Umbra::Math::Vector2f hitPoint;
    bool bHit = Umbra::Math::TestRayOBB(mRay, bounds, transform->Angle, hitPoint);
    Umbra::RenderSystem::DebugDrawCircle(mRay.Position, 0.75f, true, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(mRay.Position, mRay.Position + mRay.Direction * 500, sf::Color::Cyan);
    Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, false, sf::Color::White);
    if (bHit) {
        Umbra::RenderSystem::DebugDrawCircle(hitPoint, 2.5f, false, sf::Color::Green);
    }
}

void RaycastOBB::OnUpdate() {
    Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mBoxEntity);
    ImGui::Begin("Raycast OBB");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::Text("rect size x %f", transform->Size.x);
    ImGui::Text("rect size y     %f", transform->Size.y);
    ImGui::SliderFloat("Rect Angle", &transform->Angle, 0, 360);
    ImGui::SliderFloat("Ray Angle", &mAngle, 0, 360);
    // ImGui::SliderFloat("Radius", &mRadius, 5, 50);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> RaycastOBB::InsatiateCopy() {
    return std::make_shared<RaycastOBB>(*this);
}

void RaycastOBB::OnBeginPlay() {
    int transformX = Umbra::Random::RandomRange(-75, 75);
    int transformY = Umbra::Random::RandomRange(-75, 75);

    int sizeRX = Umbra::Random::RandomRange(30, 70);
    int sizeRY = Umbra::Random::RandomRange(30, 70);

    float angle = Umbra::Random::RandomRange(0, 360);
    mAngle      = Umbra::Random::RandomRange(0, 360);
    mBoxEntity  = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<Umbra::TransformComponent>(
        mBoxEntity, Umbra::Math::Vector2f(transformX, transformY), Umbra::Math::Vector2f(sizeRX, sizeRY), angle);
}

void RaycastOBB::OnEndPlay() {}
