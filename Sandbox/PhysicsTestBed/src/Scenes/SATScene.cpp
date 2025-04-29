#include "SATScene.h"

#include "Core/Random.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Systems/RenderSystem.h"
#include "EnginePCH.h"
#include "Game/IGameInstance.h"
#include "Input/Input.h"
#include "LandingScene.h"
#include "Math/Box.h"
#include "Math/GeometryUtils.h"
#include "Umbra.h"
#include "imgui.h"
#include <ctime>
#include <random>


void SATScene::Initialize() {
    if (GetCameraEntity() != Umbra::MAX_ENTITY) {
        GetWorld()->GetComponent<Umbra::CameraComponent>(GetCameraEntity())->SetOrthographicSize(150);
    }
    mCollisionDetector = std::make_shared<Umbra::CollisionDetector>();
}

void SATScene::OnFixedUpdated() {

    Umbra::Math::Vector2f worldMousePos   = GetWorld()->GetScreenToWorldPosition(Umbra::Input::GetMousePosition());
    Umbra::TransformComponent* transformA = GetWorld()->GetComponent<Umbra::TransformComponent>(mEntityA);
    Umbra::TransformComponent* transformB = GetWorld()->GetComponent<Umbra::TransformComponent>(mEntityB);
    Umbra::UniquePtr<Umbra::Math::Polygon> shapeA;
    Umbra::UniquePtr<Umbra::Math::Polygon> shapeB;
    if (mShapeA == Box) {
        shapeA = std::make_unique<Umbra::Math::Box>(transformA->Position, transformA->Size, transformA->Angle);
        Umbra::Math::Bounds2D boundsA(transformA->Position, transformA->Size);
        Umbra::RenderSystem::DrawDebugOrientedBox(boundsA, transformA->Angle, false, sf::Color(60, 60, 60));
    } else if (mShapeA == Polygon) {
        Umbra::Vector<Umbra::Math::Vector2f> pointsA = mPolygonA.GetVertices();
        for (int i = 0; i < pointsA.size(); i++) {
            pointsA[i] = transformA->Position + (pointsA[i].GetRotated(transformA->Angle));
        }
        shapeA = std::make_unique<Umbra::Math::Polygon>(pointsA);
        for (int i = 0; i < pointsA.size(); i++) {
            Umbra::RenderSystem::DebugDrawLine(pointsA[(i + 1) % pointsA.size()], pointsA[i], sf::Color(60, 60, 60));
        }
    }
    if (mShapeB == Box) {
        shapeB = std::make_unique<Umbra::Math::Box>(transformB->Position, transformB->Size, transformB->Angle);
        Umbra::Math::Bounds2D boundsB(transformB->Position, transformB->Size);
        Umbra::RenderSystem::DrawDebugOrientedBox(boundsB, transformB->Angle, false, sf::Color(60, 60, 60));
    } else if (mShapeB == Polygon) {
        Umbra::Vector<Umbra::Math::Vector2f> pointsB = mPolygonB.GetVertices();
        for (int i = 0; i < pointsB.size(); i++) {
            pointsB[i] = transformB->Position + (pointsB[i].GetRotated(transformB->Angle));
        }
        shapeB = std::make_unique<Umbra::Math::Polygon>(pointsB);
        for (int i = 0; i < pointsB.size(); i++) {
            Umbra::RenderSystem::DebugDrawLine(pointsB[(i + 1) % pointsB.size()], pointsB[i], sf::Color(60, 60, 60));
        }
    }

    if (mShapeA != Circle && mShapeB != Circle) {
        Umbra::Collision collision;
        bool bCollided = mCollisionDetector->CheckPolygonPolygonOverlapSAT(*shapeA, *shapeB, collision);
        if (bCollided) {
            Umbra::RenderSystem::DebugDrawLine(
                transformA->Position, transformA->Position + collision.mContactNormal * 100, sf::Color::Green);
            for (auto point : collision.GetContacts()) {
                // if (bDrawContactPoint) {
                Umbra::RenderSystem::DebugDrawCircle(point.mContactPosition, 1.5, true, sf::Color::White);
                // }
            }
            if (bDrawContactPoint) {
                Umbra::Vector<Umbra::Math::Vector2f> collisionPoint;

                int vaIx                                       = -1;
                float vaProj                                   = -Umbra::fInf;
                Umbra::Vector<Umbra::Math::Vector2f> verticesA = shapeA->GetVertices();
                for (int i = 0; i < verticesA.size(); i++) {
                    float projection = Umbra::Math::Vector2f::Dot(collision.mContactNormal, verticesA[i]);
                    if (projection >= vaProj) {
                        vaIx   = i;
                        vaProj = projection;
                    }
                }
                Umbra::Math::Vector2f vertexNextA     = verticesA[(vaIx + 1) % verticesA.size()];
                Umbra::Math::Vector2f vertexPrevA     = verticesA[(vaIx - 1) % verticesA.size()];
                Umbra::Math::Vector2f vertToNextEdgeA = (vertexNextA - verticesA[vaIx]).GetNormalized();
                Umbra::Math::Vector2f prevToVertEdgeA = (verticesA[vaIx] - vertexPrevA).GetNormalized();
                float prevProj = Umbra::Math::Vector2f::Dot(prevToVertEdgeA, collision.mContactNormal);
                float nextProj = Umbra::Math::Vector2f::Dot(vertToNextEdgeA, collision.mContactNormal);
                Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> bestEdgeA;
                if (Umbra::Math::Abs(prevProj) <= Umbra::Math::Abs(nextProj)) {
                    bestEdgeA = {vertexPrevA, verticesA[vaIx]};
                } else {
                    bestEdgeA = {verticesA[vaIx], vertexNextA};
                }

                int vbIx                                       = -1;
                float vbProj                                   = -Umbra::fInf;
                Umbra::Vector<Umbra::Math::Vector2f> verticesB = shapeB->GetVertices();
                for (int i = 0; i < verticesB.size(); i++) {
                    float projection = Umbra::Math::Vector2f::Dot(-1 * collision.mContactNormal, verticesB[i]);
                    if (projection >= vbProj) {
                        vbIx   = i;
                        vbProj = projection;
                    }
                }
                // find the edge that is perpendicular to contact normal
                Umbra::Math::Vector2f vertexNextB     = verticesB[(vbIx + 1) % verticesB.size()];
                Umbra::Math::Vector2f vertexPrevB     = verticesB[(vbIx - 1) % verticesB.size()];
                Umbra::Math::Vector2f vertToNextEdgeB = (vertexNextB - verticesB[vbIx]).GetNormalized();
                Umbra::Math::Vector2f prevToVertEdgeB = (verticesB[vbIx] - vertexPrevB).GetNormalized();
                prevProj = Umbra::Math::Vector2f::Dot(prevToVertEdgeB, -1 * collision.mContactNormal);
                nextProj = Umbra::Math::Vector2f::Dot(vertToNextEdgeB, -1 * collision.mContactNormal);
                Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> bestEdgeB;
                if (Umbra::Math::Abs(prevProj) <= Umbra::Math::Abs(nextProj)) {
                    bestEdgeB = {vertexPrevB, verticesB[vbIx]};
                } else {
                    bestEdgeB = {verticesB[vbIx], vertexNextB};
                }

                Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> referenceEdge;
                Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> incidentEdge;
                float e1Dot = Umbra::Math::Abs(
                    Umbra::Math::Vector2f::Dot((bestEdgeA.second - bestEdgeA.first), collision.mContactNormal));
                float e2Dot = Umbra::Math::Abs(
                    Umbra::Math::Vector2f::Dot((bestEdgeB.second - bestEdgeB.first), -1 * collision.mContactNormal));

                bool bFlipInc = false;
                if (e1Dot <= e2Dot) {
                    bFlipInc      = false;
                    referenceEdge = bestEdgeA;
                    incidentEdge  = bestEdgeB;
                } else {
                    referenceEdge = bestEdgeB;
                    incidentEdge  = bestEdgeA;
                    bFlipInc      = true;
                }

                Umbra::Math::Vector2f refEdge   = (referenceEdge.second - referenceEdge.first).GetNormalized();
                Umbra::Math::Vector2f refNormal = Umbra::Math::Vector2f(refEdge.y, -refEdge.x);
                if (Umbra::Math::Vector2f::Dot(collision.mContactNormal, refNormal) < 0) {
                    refNormal = -1 * refNormal; // Flip the normal to make sure it points in the correct direction
                }

                float refC1 = Umbra::Math::Vector2f::Dot(refEdge, referenceEdge.first);
                float refC2 = Umbra::Math::Vector2f::Dot(refEdge, referenceEdge.second) * -1;
                Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> clipped = incidentEdge;
                if (!mCollisionDetector->Clip(refEdge, clipped, refC1)) {
                } else if (!mCollisionDetector->Clip(-1 * refEdge, clipped, refC2)) {
                } else {
                    if (bFlipInc) {
                        refNormal *= -1;
                    }
                    float refDepth = Umbra::Math::Vector2f::Dot(refNormal, referenceEdge.first);
                    for (auto& point : {clipped.first, clipped.second}) {
                        float depth = Umbra::Math::Vector2f::Dot(refNormal, point) - refDepth;
                        if (depth <= 0.0f) {
                            collisionPoint.emplace_back(point);
                        }
                    }


                    Umbra::RenderSystem::DebugDrawCircle(clipped.first, 2.5, false, sf::Color::White);
                    Umbra::RenderSystem::DebugDrawCircle(clipped.second, 2.5, false, sf::Color::White);
                }

                Umbra::RenderSystem::DebugDrawCircle(verticesA[vaIx], 2.5, false, sf::Color::Red);
                Umbra::RenderSystem::DebugDrawCircle(verticesB[vbIx], 2.5, false, sf::Color::Green);
                Umbra::RenderSystem::DebugDrawCircle(vertexPrevA, 2.5, false, sf::Color::Blue);
                Umbra::RenderSystem::DebugDrawCircle(vertexNextA, 2.5, false, sf::Color::Yellow);
                Umbra::RenderSystem::DebugDrawCircle(vertexPrevB, 2.5, false, sf::Color::Blue);
                Umbra::RenderSystem::DebugDrawCircle(vertexNextB, 2.5, false, sf::Color::Yellow);
                Umbra::RenderSystem::DebugDrawLine(referenceEdge.first, referenceEdge.second, sf::Color::Red);
                Umbra::RenderSystem::DebugDrawLine(incidentEdge.first, incidentEdge.second, sf::Color::Green);
            }


            // // find the edge from the vertex such that it is most perpendicular to the collision normal
            // Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> prevVertEdgeA = {
            //     verticesA[vaIx - 1 % verticesA.size()], verticesA[vaIx]};
            // Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> nextVertEdgeA = {
            //     verticesA[vaIx], verticesA[vaIx + 1 % verticesA.size()]};
            // Umbra::Pair<Umbra::Math::Vector2f, Umbra::Math::Vector2f> bestEdgeA;
            // if ()
        }
    }


    if (mShapeA == Box) {
        shapeA = std::make_unique<Umbra::Math::Box>(transformA->Position, transformA->Size, transformA->Angle);
    } else if (mShapeA == Polygon) {
        Umbra::Vector<Umbra::Math::Vector2f> pointsA = mPolygonA.GetVertices();
        for (int i = 0; i < pointsA.size(); i++) {
            pointsA[i] = transformA->Position + (pointsA[i].GetRotated(transformA->Angle));
        }
        shapeA = std::make_unique<Umbra::Math::Polygon>(pointsA);
    }
    if (mShapeB == Box) {
        shapeB = std::make_unique<Umbra::Math::Box>(transformB->Position, transformB->Size, transformB->Angle);
    } else if (mShapeB == Polygon) {
        Umbra::Vector<Umbra::Math::Vector2f> pointsB = mPolygonB.GetVertices();
        for (int i = 0; i < pointsB.size(); i++) {
            pointsB[i] = transformB->Position + (pointsB[i].GetRotated(transformB->Angle));
        }
        shapeB = std::make_unique<Umbra::Math::Polygon>(pointsB);
    }


    Umbra::Math::Vector2f minAxis;
    bool flip = false;
    if (bShowProjections) {
        if (mShapeA != Circle) {
            Umbra::Vector<Umbra::Math::Vector2f> normals = shapeA->GetNormals();
            for (auto normal : normals) {
                Umbra::RenderSystem::DebugDrawLine(Umbra::Math::Vector2f(0, 0), normal * 100, sf::Color(0, 75, 75));
            }
        }
        if (mShapeB != Circle) {
            Umbra::Vector<Umbra::Math::Vector2f> normals = shapeB->GetNormals();
            for (auto normal : normals) {
                Umbra::RenderSystem::DebugDrawLine(Umbra::Math::Vector2f(0, 0), normal * 100, sf::Color(75, 0, 75));
            }
        }
        Umbra::Vector<Umbra::Math::Vector2f> normalsA = shapeA->GetNormals();
        Umbra::Vector<Umbra::Math::Vector2f> normalsB = shapeB->GetNormals();
        Umbra::Vector<Umbra::Math::Vector2f> axes;
        axes.insert(axes.end(), normalsA.begin(), normalsA.end());
        axes.insert(axes.end(), normalsB.begin(), normalsB.end());
        float minOverLap = Umbra::fInf;
        // For every axis project both shape and find if any axis exist which has no overlap
        // if overlap is not found objects are separated
        // else find the axis with minimum overlap to find minimum translation vector
        for (Umbra::Math::Vector2f axis : normalsA) {
            Umbra::Math::Polygon::Projection projectionA = shapeA->GetProjectionOntoAxis(axis);
            Umbra::Math::Polygon::Projection projectionB = shapeB->GetProjectionOntoAxis(axis);
            // Umbra::RenderSystem::DebugDrawLine(axis * projectionB.Min, axis * projectionB.Max, sf::Color::Red);
            // Umbra::RenderSystem::DebugDrawLine(axis * projectionA.Min, axis * projectionA.Max, sf::Color::Blue);
            if (projectionA.Min > projectionB.Max || projectionA.Max < projectionB.Min) {
                // axis is the separating axis theorem
            } else {
                Umbra::RenderSystem::DebugDrawLine(axis * Umbra::Math::Max(projectionA.Min, projectionB.Min),
                    axis * Umbra::Math::Min(projectionA.Max, projectionB.Max), sf::Color::Yellow);
                float overlap = Umbra::Math::Min(projectionA.Max, projectionB.Max)
                              - Umbra::Math::Max(projectionA.Min, projectionB.Min);
                if (overlap < minOverLap) {
                    minOverLap = overlap;
                    minAxis    = axis;
                    flip       = false;
                }
            }
        }
        for (Umbra::Math::Vector2f axis : normalsB) {
            Umbra::Math::Polygon::Projection projectionA = shapeA->GetProjectionOntoAxis(axis);
            Umbra::Math::Polygon::Projection projectionB = shapeB->GetProjectionOntoAxis(axis);
            // Umbra::RenderSystem::DebugDrawLine(axis * projectionA.Min, axis * projectionA.Max, sf::Color::Blue);
            // Umbra::RenderSystem::DebugDrawLine(axis * projectionB.Min, axis * projectionB.Max, sf::Color::Red);
            if (projectionA.Min > projectionB.Max || projectionA.Max < projectionB.Min) {
                // axis is the separating axis theorem
            } else {
                // Umbra::RenderSystem::DebugDrawLine(axis * Umbra::Math::Max(projectionA.Min, projectionB.Min),
                // axis * Umbra::Math::Min(projectionA.Max, projectionB.Max), sf::Color::Yellow);
                float overlap = Umbra::Math::Min(projectionA.Max, projectionB.Max)
                              - Umbra::Math::Max(projectionA.Min, projectionB.Min);
                if (overlap < minOverLap) {
                    minOverLap = overlap;
                    minAxis    = axis;
                    flip       = true;
                }
            }
        }
    }

    Umbra::RenderSystem::DebugDrawCircle(worldMousePos, 2.f, true, sf::Color::White);
    Umbra::RenderSystem::DebugDrawCircle(transformA->Position, 2.f, true, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(transformA->Position,
        transformA->Position + Umbra::Math::Vector2f(1, 0).GetRotated(transformA->Angle) * 10, sf::Color::Cyan);
    Umbra::RenderSystem::DebugDrawLine(
        transformA->Position, transformA->Position + minAxis * 70, flip ? sf::Color::Cyan : sf::Color::White);
    Umbra::RenderSystem::DebugDrawCircle(transformB->Position, 2.f, true, sf::Color::Magenta);
    Umbra::RenderSystem::DebugDrawLine(transformB->Position,
        transformB->Position + Umbra::Math::Vector2f(1, 0).GetRotated(transformB->Angle) * 10, sf::Color::Magenta);
}

void SATScene::OnUpdate() {
    ImGui::Begin("SAT Demo");
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
    if (ImGui::Button("Restart")) {
        GetSceneManager().GoToScene(this->GetSceneID());
    }
    {
        ImGui::BeginChild("Body A", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
        Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mEntityA);
        if (transform != nullptr) {
            ImGui::Text("BODY A");
            int pos[2] = {transform->Position.x, transform->Position.y};
            if (ImGui::InputInt2("Position", pos)) {
                transform->Position.x = pos[0];
                transform->Position.y = pos[1];
            }
            ImGui::SliderFloat("Angle", &transform->Angle, 0, 360);
            int selectedShape = mShapeA;
            ImGui::Text("Select Shape:");
            ImGui::RadioButton("Circle", &selectedShape, ShapeType::Circle);
            ImGui::SameLine();
            ImGui::RadioButton("Box", &selectedShape, ShapeType::Box);
            ImGui::SameLine();
            ImGui::RadioButton("Polygon", &selectedShape, ShapeType::Polygon);
            mShapeA = (ShapeType) selectedShape;
        }
        ImGui::EndChild();
    }
    {
        ImGui::BeginChild("Body B", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
        Umbra::TransformComponent* transform = GetWorld()->GetComponent<Umbra::TransformComponent>(mEntityB);
        if (transform != nullptr) {
            ImGui::Text("BODY B");
            int pos[2] = {transform->Position.x, transform->Position.y};
            if (ImGui::InputInt2("Position", pos)) {
                transform->Position.x = pos[0];
                transform->Position.y = pos[1];
            }
            ImGui::SliderFloat("Angle", &transform->Angle, 0, 360);

            int selectedShape = mShapeB;
            ImGui::Text("Select Shape:");
            ImGui::RadioButton("Circle", &selectedShape, ShapeType::Circle);
            ImGui::SameLine();
            ImGui::RadioButton("Box", &selectedShape, ShapeType::Box);
            ImGui::SameLine();
            ImGui::RadioButton("Polygon", &selectedShape, ShapeType::Polygon);
            mShapeB = (ShapeType) selectedShape;
        }
        ImGui::EndChild();
    }
    ImGui::BeginChild("Projection", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
    ImGui::Checkbox("show Projections", &bShowProjections);
    ImGui::ColorButton(
        "##NearVecColor1", ImVec4(0.0f, 0.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
    ImGui::SameLine(0, 20);
    ImGui::Text("Shape A");
    ImGui::SameLine(0, 20);
    ImGui::ColorButton(
        "##NearVecColor2", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
    ImGui::SameLine(0, 20);
    ImGui::Text("Shape B");
    ImGui::SameLine(0, 20);
    ImGui::ColorButton(
        "##NearVecColor3", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
    ImGui::SameLine(0, 20);
    ImGui::Text("Overlap");
    ImGui::EndChild();
    ImGui::BeginChild("Contact", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
    ImGui::Checkbox("show contacts", &bDrawContactPoint);
    ImGui::ColorButton(
        "##NearVecColor1", ImVec4(0.0f, 0.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
    ImGui::SameLine(0, 20);
    ImGui::Text("incident edge");
    ImGui::SameLine(0, 20);
    ImGui::ColorButton(
        "##NearVecColor2", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
    ImGui::SameLine(0, 20);
    ImGui::Text("reference edge");
    ImGui::SameLine(0, 20);
    ImGui::ColorButton(
        "##NearVecColor3", ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
    ImGui::SameLine(0, 20);
    ImGui::EndChild();
    if (ImGui::Button("Main Menu")) {
        GetSceneManager().GoToScene("Scene0");
    }
    ImGui::End();
}

Umbra::SharedPtr<Umbra::Scene> SATScene::InsatiateCopy() {
    return std::make_shared<SATScene>(*this);
}

void SATScene::OnBeginPlay() {
    mEntityA = GetWorld()->CreateEntity();
    mEntityB = GetWorld()->CreateEntity();
    {
        int transformX = Umbra::Random::RandomRange(-75, 75);
        int transformY = Umbra::Random::RandomRange(-75, 75);
        int sizeRX     = Umbra::Random::RandomRange(30, 70);
        int sizeRY     = Umbra::Random::RandomRange(30, 70);
        float angle    = Umbra::Random::RandomRange(0, 360);
        GetWorld()->AddComponent<Umbra::TransformComponent>(
            mEntityA, Umbra::Math::Vector2f(transformX, transformY), Umbra::Math::Vector2f(sizeRX, sizeRY), angle);
    }
    {
        int transformX = Umbra::Random::RandomRange(-75, 75);
        int transformY = Umbra::Random::RandomRange(-75, 75);
        int sizeRX     = Umbra::Random::RandomRange(30, 70);
        int sizeRY     = Umbra::Random::RandomRange(30, 70);
        float angle    = Umbra::Random::RandomRange(0, 360);
        GetWorld()->AddComponent<Umbra::TransformComponent>(
            mEntityB, Umbra::Math::Vector2f(transformX, transformY), Umbra::Math::Vector2f(sizeRX, sizeRY), angle);
    }
    {
        int randomEdge = Umbra::Random::RandomRange(3, 12);
        int sizeR      = Umbra::Random::RandomRange(30, 70);
        mPolygonA      = Umbra::Math::Polygon(GetRandomPolygon(randomEdge, sizeR));
    }
    {
        int randomEdge = Umbra::Random::RandomRange(3, 12);
        int sizeR      = Umbra::Random::RandomRange(30, 70);
        mPolygonB      = Umbra::Math::Polygon(GetRandomPolygon(randomEdge, sizeR));
    }
}

void SATScene::OnEndPlay() {}

Umbra::Vector<Umbra::Math::Vector2f> SATScene::GetRandomPolygon(int _edges, int _sizeRadius) {
    // Generate two lists of random X and Y coordinates
    Umbra::Vector<float> xPool;
    Umbra::Vector<float> yPool;
    for (int i = 0; i < _edges; i++) {
        xPool.emplace_back(Umbra::Random::RandomRange(-_sizeRadius, _sizeRadius));
        yPool.emplace_back(Umbra::Random::RandomRange(-_sizeRadius, _sizeRadius));
    }
    // Sort them
    std::sort(xPool.begin(), xPool.end());
    std::sort(yPool.begin(), yPool.end());

    // Isolate the extreme points
    float minX = xPool.front();
    float maxX = xPool.back();
    float minY = xPool.front();
    float maxY = xPool.back();

    // Divide the interior points into two chains & Extract the vector components
    float lastTop = minX;
    float lastBot = minX;
    Umbra::Vector<float> xVec;
    Umbra::Vector<float> yVec;

    for (int i = 0; i < _edges - 1; i++) {
        float x = xPool[i];
        if (Umbra::Random::GetRandom() > 0.5f) {
            xVec.emplace_back(x - lastTop);
            lastTop = x;
        } else {
            xVec.emplace_back(lastBot - x);
            lastBot = x;
        }
    }
    xVec.emplace_back(maxX - lastTop);
    xVec.emplace_back(lastBot - maxX);

    float lastLeft  = minY;
    float lastRight = minY;
    for (int i = 0; i < _edges - 1; i++) {
        float y = yPool[i];
        if (Umbra::Random::GetRandom() > 0.5f) {
            yVec.emplace_back(y - lastLeft);
            lastLeft = y;
        } else {
            yVec.emplace_back(lastRight - y);
            lastRight = y;
        }
    }
    yVec.emplace_back(maxY - lastLeft);
    yVec.emplace_back(lastRight - maxY);
    // Randomly pair up the X- and Y-components
    std::shuffle(yVec.begin(), yVec.end(), std::mt19937(std::time(nullptr)));

    // Combine the paired up components into vectors
    Umbra::Vector<Umbra::Math::Vector2f> vectors;
    for (int i = 0; i < _edges; i++) {
        vectors.emplace_back(Umbra::Math::Vector2f(xVec[i], yVec[i]));
    }

    // Sort the vectors by angle
    std::sort(vectors.begin(), vectors.end(), [](const Umbra::Math::Vector2f& a, const Umbra::Math::Vector2f& b) {
        return Umbra::Math::Atan2(a.y, a.x) < Umbra::Math::Atan2(b.y, b.x);
    });

    // Lay them end-to-end
    float x = 0, y = 0;
    float minPolygonX = 0;
    float minPolygonY = 0;
    Umbra::Vector<Umbra::Math::Vector2f> points;
    for (int i = 0; i < _edges; i++) {
        points.emplace_back(Umbra::Math::Vector2f(x, y));
        x += vectors[i].x;
        y += vectors[i].y;
        minPolygonX = Umbra::Math::Min(minPolygonX, x);
        minPolygonY = Umbra::Math::Min(minPolygonY, y);
    }

    // Compute centroid
    Umbra::Math::Vector2f centroid(0.0f, 0.0f);
    for (const auto& p : points) {
        centroid.x += p.x;
        centroid.y += p.y;
    }
    centroid.x /= static_cast<float>(points.size());
    centroid.y /= static_cast<float>(points.size());

    // Shift all points so that centroid is at (0, 0)
    for (auto& p : points) {
        p.x -= centroid.x;
        p.y -= centroid.y;
    }
    return points;
}
