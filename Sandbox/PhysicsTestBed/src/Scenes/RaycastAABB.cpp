#include "RaycastAABB.h"

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

void RaycastAABB::Initialize() {
    if (GetCameraEntity() != Umbra::MAX_ENTITY) {
        GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(200);
    }
}

void RaycastAABB::OnFixedUpdated() {
    mRay.Position  = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    mRay.Direction = Umbra::Math::Vector2f(1, 0).GetRotated(mAngle);
    Umbra::Math::Bounds2D RectAABB(mPoint, mSize);
    Umbra::Math::Vector2f outPoint;
    bool bHit = Umbra::Math::TestRayAABB(mRay, RectAABB, outPoint);


    Umbra::RenderSystem::DebugDrawLine(mRay.Position, mRay.Position + mRay.Direction * 500, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawCircle(mRay.Position, 2, true, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(Umbra::Math::Vector2f(RectAABB.Min().x - 500, RectAABB.Min().y),
        Umbra::Math::Vector2f(RectAABB.Min().x + 500, RectAABB.Min().y), sf::Color::Yellow);
    Umbra::RenderSystem::DebugDrawLine(Umbra::Math::Vector2f(RectAABB.Max().x - 500, RectAABB.Max().y),
        Umbra::Math::Vector2f(RectAABB.Max().x + 500, RectAABB.Max().y), sf::Color::Yellow);
    Umbra::RenderSystem::DebugDrawLine(Umbra::Math::Vector2f(RectAABB.Min().x, RectAABB.Min().y - 500),
        Umbra::Math::Vector2f(RectAABB.Min().x, RectAABB.Min().y + 500), sf::Color::Yellow);
    Umbra::RenderSystem::DebugDrawLine(Umbra::Math::Vector2f(RectAABB.Max().x, RectAABB.Max().y - 500),
        Umbra::Math::Vector2f(RectAABB.Max().x, RectAABB.Max().y + 500), sf::Color::Yellow);
    Umbra::RenderSystem::DrawDebugBox(RectAABB, false, bHit ? sf::Color::Red : sf::Color::White);

    // p = r + dt
    // t = (p-r)/d
    // tx = (px - rx)/dx
    // ty = (py - ry)/dy
    float tMinX = 0;
    float tMinY = 0;
    float tMaxX = 0;
    float tMaxY = 0;
    if (mRay.Direction.x != 0) {
        tMinX                     = (RectAABB.Min().x - mRay.Position.x) / mRay.Direction.x;
        Umbra::Math::Vector2f rTx = mRay.Position + mRay.Direction * tMinX;
        Umbra::RenderSystem::DebugDrawCircle(rTx, 3, false, sf::Color::Cyan);
    }
    if (mRay.Direction.y != 0) {
        tMinY                     = (RectAABB.Min().y - mRay.Position.y) / mRay.Direction.y;
        Umbra::Math::Vector2f rTy = mRay.Position + mRay.Direction * tMinY;
        Umbra::RenderSystem::DebugDrawCircle(rTy, 3, true, sf::Color::Cyan);
    }
    if (mRay.Direction.y != 0) {
        tMaxX                     = (RectAABB.Max().x - mRay.Position.x) / mRay.Direction.x;
        Umbra::Math::Vector2f rTx = mRay.Position + mRay.Direction * tMaxX;
        Umbra::RenderSystem::DebugDrawCircle(rTx, 3, false, sf::Color::Magenta);
    }
    if (mRay.Direction.y != 0) {
        tMaxY                     = (RectAABB.Max().y - mRay.Position.y) / mRay.Direction.y;
        Umbra::Math::Vector2f rTy = mRay.Position + mRay.Direction * tMaxY;
        Umbra::RenderSystem::DebugDrawCircle(rTy, 3, true, sf::Color::Magenta);
    }
    float nearestX  = Umbra::Math::Min(tMinX, tMaxX);
    float nearestY  = Umbra::Math::Min(tMinY, tMaxY);
    float furthestX = Umbra::Math::Max(tMinX, tMaxX);
    float furthestY = Umbra::Math::Max(tMinY, tMaxY);
    // ray hit entry
    float furthestNearPoint = Umbra::Math::Max(nearestX, nearestY);
    // ray hit exit
    float nearestFarPoint = Umbra::Math::Min(furthestX, furthestY);
    if (bHit) {
        Umbra::RenderSystem::DebugDrawCircle(outPoint, 2, true, sf::Color::Black);
        Umbra::RenderSystem::DebugDrawLine(
            mRay.Position + mRay.Direction * furthestNearPoint, mRay.Position + mRay.Direction * nearestFarPoint);
    }
}

void RaycastAABB::OnUpdate() {
    ImGui::Begin("AABB Raycast");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImVec2 boxSize(12, 12);

    float spacing = 6.0f;

    ImGui::ColorButton("##NearVecColor", ImVec4(0.0f, 1.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, boxSize);
    ImGui::SameLine(0, spacing);
    ImGui::Text("unfilled -> min X filled -> min Y");

    ImGui::ColorButton("##FarSquareColor", ImVec4(1.0f, 0.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, boxSize);
    ImGui::SameLine(0, spacing);
    ImGui::Text("unfilled -> max X filled -> max Y");

    ImGui::SliderFloat("Angle", &mAngle, 0, 360);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> RaycastAABB::InsatiateCopy() {
    return std::make_shared<RaycastAABB>(*this);
}

void RaycastAABB::OnBeginPlay() {
    float randomX = Umbra::Random::RandomRange(-75, 75);
    float randomY = Umbra::Random::RandomRange(-75, 75);
    mPoint        = Umbra::Math::Vector2f(randomX, randomY);
    randomX       = Umbra::Random::RandomRange(0, 90);
    randomY       = Umbra::Random::RandomRange(0, 90);
    mSize         = Umbra::Math::Vector2f(randomX, randomY);
    mAngle        = Umbra::Random::RandomRange(0, 360);
}

void RaycastAABB::OnEndPlay() {}
