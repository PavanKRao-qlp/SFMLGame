#include "MainScene.h"

#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
#include "Input/Input.h"
#include "Umbra.h"
#include "imgui.h"

void MainScene::Initialize() {
    CreateDefaultCamera(100.0f);
}

void MainScene::OnFixedUpdate() {}

void MainScene::OnUpdate() {
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Physics Engine Demo");
    if (ImGui::Button("Particle Object : Integration")) {
        GetSceneManager().GoToScene("ParticleIntegration");
    }
    if (ImGui::Button("Particle Object : Force")) {
        GetSceneManager().GoToScene("ForceAndTorque");
    }
    if (ImGui::Button("Shape Representation")) {
        GetSceneManager().GoToScene("ShapeRepresentation");
    }
    if (ImGui::Button("Bounding Volume and Overlap")) {
        GetSceneManager().GoToScene("BoundingVolume");
    }
    if (ImGui::Button("Narrow Phase Collisions")) {
        GetSceneManager().GoToScene("NarrowPhase");
    }
    if (ImGui::Button("Broadphase AABB Tree")) {
        GetSceneManager().GoToScene("BroadphaseTree");
    }
    ImGui::End();

    Umbra::RenderSystem::DebugDrawCircle(Umbra::Math::Vector2f(0.0f, 0.0f), mCircleRadius, true, Umbra::Color::White);
}

Umbra::SharedPtr<Umbra::Scene> MainScene::InstantiateCopy() {
    return std::make_shared<MainScene>(*this);
}

void MainScene::OnBeginPlay() {}

void MainScene::OnEndPlay() {}
