#include "CircleOBBScene.h"

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

void CircleOBBScene::Initialize() {
    CreateDefaultCamera(200.0f);
}

void CircleOBBScene::OnFixedUpdate() {

    Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mBoxEntity);
    Umbra::Math::Vector2f worldMousePos  = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::Math::Bounds2D bounds         = Umbra::Math::Bounds2D(transform->Position, transform->Size);
    Umbra::Math::Vector2f pointInOBB =
        Umbra::Math::GetClosestPointInsideOrientedBound(bounds, transform->Angle, worldMousePos);
    Umbra::Math::Vector2f pointOnOBB =
        Umbra::Math::GetClosestPointOnOrientedBoundEdge(bounds, transform->Angle, worldMousePos);
    mDistance       = (pointInOBB - worldMousePos).Magnitude();
    bool bColliding = mDistance <= mRadius;
    {
        Umbra::Math::Vector2f halfSize = transform->Size * 0.5f;
        Umbra::Vector<Umbra::Math::Vector2f> corners;
        Umbra::Vector<Umbra::Math::Vector2f> normals;
        corners.emplace_back(
            transform->Position + Umbra::Math::Vector2f(halfSize.x, halfSize.y).GetRotated(transform->Angle));
        corners.emplace_back(
            transform->Position + Umbra::Math::Vector2f(halfSize.x, -halfSize.y).GetRotated(transform->Angle));
        corners.emplace_back(
            transform->Position + Umbra::Math::Vector2f(-halfSize.x, -halfSize.y).GetRotated(transform->Angle));
        corners.emplace_back(
            transform->Position + Umbra::Math::Vector2f(-halfSize.x, halfSize.y).GetRotated(transform->Angle));
        for (int i = 0; i < corners.size(); i++) {

            Umbra::Math::Vector2f edge   = (corners[(i + 1) % corners.size()] - corners[i]);
            Umbra::Color color              = Umbra::Color::Red;
            Umbra::Math::Vector2f normal = Umbra::Math::Vector2f::Perpendicular(edge).GetNormalized();
            normals.emplace_back(normal);
            if (i == 0) {
                color = Umbra::Color::Green;
            } else if (i == 1) {
                color = Umbra::Color::Blue;
            } else if (i == 2) {
                color = Umbra::Color::Yellow;
            } else if (i == 3) {
                color = Umbra::Color::Red;
            }
            Umbra::Math::Vector2f edgeMid = corners[i] + (edge * 0.5f);
            Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(corners[i], 2.5f, false, color);
            Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(edgeMid, edgeMid + (normal * 10), color);
        }
        for (int n = 0; n < normals.size(); n++) {
            Umbra::Color normalColor = Umbra::Color::Red;
            if (n == 0) {
                normalColor = Umbra::Color::Green;
            } else if (n == 1) {
                normalColor = Umbra::Color::Blue;
            } else if (n == 2) {
                normalColor = Umbra::Color::Yellow;
            } else if (n == 3) {
                normalColor = Umbra::Color::Red;
            }
            Umbra::Math::Vector2f normal = normals[n];
            for (int i = 0; i < corners.size(); i++) {
                float projection = Umbra::Math::Vector2f::Dot(corners[i], normal);
                // s = AP.AB/AB.AB since AB is normalized it will be 1
                Umbra::Math::Vector2f projectionOnNormalScaled = normal * projection;

                Umbra::Color color = Umbra::Color::Red;
                if (i == 0) {
                    color = Umbra::Color::Green;
                } else if (i == 1) {
                    color = Umbra::Color::Blue;
                } else if (i == 2) {
                    color = Umbra::Color::Yellow;
                } else if (i == 3) {
                    color = Umbra::Color::Red;
                }
                Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(
                    projectionOnNormalScaled, (n == 0 || n == 1) ? 2.5f : 3.5f, n == 0 || n == 1, normalColor);
                Umbra::Math::Vector2f projectionGuide = projectionOnNormalScaled - corners[i];
                Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(corners[i], corners[i] + projectionGuide, color);
            }
            float mouseProjection                               = Umbra::Math::Vector2f::Dot(worldMousePos, normal);
            Umbra::Math::Vector2f mouseProjectionOnNormalScaled = normal * mouseProjection;

            Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(Umbra::Math::Vector2f(0, 0), normal * 500, normalColor);
            Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(
                mouseProjectionOnNormalScaled, (n == 0 || n == 1) ? 2.5f : 3.5f, n == 0 || n == 1, normalColor);
            Umbra::ServiceLocator::GetRenderService()->DebugDrawLine(
                worldMousePos, worldMousePos - mouseProjectionOnNormalScaled, Umbra::Color::Cyan);
        }

        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(transform->Position, 0.75f, true, Umbra::Color::White);
        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pointInOBB, 3.f, true, Umbra::Color::Magenta);
        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pointOnOBB, 2.5f, true, Umbra::Color::White);
        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(worldMousePos, 1.5f, true, Umbra::Color::Cyan);
        Umbra::ServiceLocator::GetRenderService()->DebugDrawOrientedBox(bounds, transform->Angle, false, Umbra::Color::White);
        Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(
            worldMousePos, mRadius, false, !bColliding ? Umbra::Color::Red : Umbra::Color::Green);
    }

    // Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(worldMousePos, 1.5f, true, Umbra::Color::Red);
    // Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(mPoint, 1.f, true, Umbra::Color::White);
    // Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pointOnBox, 1.2f, true, Umbra::Color::Blue);
    // Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(pointWithinBox, 1.f, true, Umbra::Color::Green);
    // bool bColliding = (mDistance <= mRadius);
    // Umbra::ServiceLocator::GetRenderService()->DebugDrawCircle(worldMousePos, mRadius, false, bColliding ? Umbra::Color::Green :
    // Umbra::Color::Red);
}

void CircleOBBScene::OnUpdate() {
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
    Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mBoxEntity);
    if (Umbra::Input::GetKey(Umbra::KeyBoard::Escape)) {
        GetGameInstance().QuitApplication();
    }
    ImGui::Begin("Circle OBB overlap distance");
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    ImGui::Text("rect size x %f", transform->Size.x);
    ImGui::Text("rect size y     %f", transform->Size.y);
    ImGui::SliderFloat("Angle", &transform->Angle, 0, 360);
    ImGui::SliderFloat("Radius", &mRadius, 10, 50);
    ImGui::Text("distance     %f", mDistance);
    // ImGui::SliderFloat("Radius", &mRadius, 5, 50);
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> CircleOBBScene::InstantiateCopy() {
    return std::make_shared<CircleOBBScene>(*this);
}

void CircleOBBScene::OnBeginPlay() {

    int transformX = Umbra::Random::RandomRange(-75, 75);
    int transformY = Umbra::Random::RandomRange(-75, 75);

    int sizeRX = Umbra::Random::RandomRange(30, 70);
    int sizeRY = Umbra::Random::RandomRange(30, 70);

    float angle = Umbra::Random::RandomRange(0, 360);
    mRadius     = Umbra::Random::RandomRange(10, 50);

    mBoxEntity = GetWorld()->CreateEntity();
    GetWorld()->AddComponent<Umbra::TransformComponent>(
        mBoxEntity, Umbra::Math::Vector2f(transformX, transformY), Umbra::Math::Vector2f(sizeRX, sizeRY), angle);
    // GetWorld()->AddComponent<Umbra::SpriteComponent>(mBoxEntity, Umbra::Color::Red);
}

void CircleOBBScene::OnEndPlay() {}
