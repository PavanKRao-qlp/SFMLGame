#include "RaycastOBB.h"

#include "Core/Random.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/GeometryUtils.h"
#include "Graphics/Color.h"
#include "Umbra.h"
#include "imgui.h"

void RaycastOBB::Initialize() {
    CreateDefaultCamera(200.0f);
}

void RaycastOBB::OnFixedUpdate() {
    mRay.Position                        = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    mRay.Direction                       = Umbra::Math::Vector2f(1, 0).GetRotated(mAngle);
    Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mBoxEntity);
    Umbra::Math::Bounds2D bounds         = Umbra::Math::Bounds2D(transform->Position, transform->Size);
    Umbra::Math::Vector2f hitPoint;
    bool bHit = Umbra::Math::TestRayOBB(mRay, bounds, transform->Angle, hitPoint);
    {
        float angle                      = Umbra::Math::DegreeToRadian(transform->Angle);
        Umbra::Math::Vector2f boxXBasis  = Umbra::Math::Vector2f(Umbra::Math::Cos(angle), Umbra::Math::Sin(angle));
        Umbra::Math::Vector2f boxYBasis  = Umbra::Math::Vector2f(-Umbra::Math::Sin(angle), Umbra::Math::Cos(angle));
        Umbra::Math::Vector2f boxToPoint = (mRay.Position - bounds.Center);

        Umbra::Math::Vector2f rayPosInLocal = Umbra::Math::Vector2f(
            Umbra::Math::Vector2f::Dot(boxToPoint, boxXBasis), Umbra::Math::Vector2f::Dot(boxToPoint, boxYBasis));
        Umbra::Math::Vector2f rayDirInLocal =
            Umbra::Math::Vector2f(Umbra::Math::Vector2f::Dot(mRay.Direction, boxXBasis),
                Umbra::Math::Vector2f::Dot(mRay.Direction, boxYBasis));
        // must  be in local (centered at 0) space
        Umbra::Math::Vector2f localMin = -1 * bounds.Size * 0.5f;
        Umbra::Math::Vector2f localMax = 1 * bounds.Size * 0.5f;

        Umbra::Math::Vector2f rayToMin = (localMin - rayPosInLocal);
        float tMinX                    = (rayToMin.x) / (rayDirInLocal.x);
        float tMinY                    = (rayToMin.y) / (rayDirInLocal.y);
        // (p - ro) when p = max
        Umbra::Math::Vector2f rayToMax = (localMax - rayPosInLocal);
        float tMaxX                    = (rayToMax.x) / (rayDirInLocal.x);
        float tMaxY                    = (rayToMax.y) / (rayDirInLocal.y);

        Umbra::RenderSystem::DebugDrawLine(bounds.Center + (boxXBasis * -500) + (boxYBasis * localMin.y),
            bounds.Center + (boxXBasis * 500) + (boxYBasis * localMin.y), Umbra::Color::Blue);
        Umbra::RenderSystem::DebugDrawLine(bounds.Center + (boxXBasis * -500) + (boxYBasis * localMax.y),
            bounds.Center + (boxXBasis * 500) + (boxYBasis * localMax.y), Umbra::Color::Blue);
        Umbra::RenderSystem::DebugDrawLine(bounds.Center + (boxXBasis * localMin.x) + (boxYBasis * -500),
            bounds.Center + (boxXBasis * localMin.x) + (boxYBasis * 500), Umbra::Color::Blue);
        Umbra::RenderSystem::DebugDrawLine(bounds.Center + (boxXBasis * localMax.x) + (boxYBasis * -500),
            bounds.Center + (boxXBasis * localMax.x) + (boxYBasis * 500), Umbra::Color::Blue);
        Umbra::RenderSystem::DebugDrawCircle(mRay.Position + mRay.Direction * tMinX, 3.5f, false, Umbra::Color::Cyan);
        Umbra::RenderSystem::DebugDrawCircle(mRay.Position + mRay.Direction * tMinY, 3.5f, true, Umbra::Color::Cyan);
        Umbra::RenderSystem::DebugDrawCircle(mRay.Position + mRay.Direction * tMaxX, 3.5f, false, Umbra::Color::Magenta);
        Umbra::RenderSystem::DebugDrawCircle(mRay.Position + mRay.Direction * tMaxY, 3.5f, true, Umbra::Color::Magenta);
    }
    Umbra::RenderSystem::DebugDrawCircle(mRay.Position, 0.75f, true, Umbra::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(mRay.Position, mRay.Position + mRay.Direction * 500, Umbra::Color::Cyan);
    Umbra::RenderSystem::DrawDebugOrientedBox(bounds, transform->Angle, false, Umbra::Color::White);
    if (bHit) {
        Umbra::RenderSystem::DebugDrawCircle(hitPoint, 2.f, true, Umbra::Color::Green);
    }
}

void RaycastOBB::OnUpdate() {
    Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mBoxEntity);
    ImGui::Begin("Raycast OBB");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImVec2 boxSize(12, 12);

    float spacing = 6.0f;
    ImGui::Text("rect size x %f", transform->Size.x);
    ImGui::Text("rect size y     %f", transform->Size.y);
    ImGui::SliderFloat("Rect Angle", &transform->Angle, 0, 360);
    ImGui::SliderFloat("Ray Angle", &mAngle, 0, 360);
    ImGui::ColorButton("##NearVecColor", ImVec4(0.0f, 1.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, boxSize);
    ImGui::SameLine(0, spacing);
    ImGui::Text("unfilled -> min X filled -> min Y");

    ImGui::ColorButton("##FarSquareColor", ImVec4(1.0f, 0.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, boxSize);
    ImGui::SameLine(0, spacing);
    ImGui::Text("unfilled -> max X filled -> max Y");
    // ImGui::SliderFloat("Radius", &mRadius, 5, 50);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> RaycastOBB::InstantiateCopy() {
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
