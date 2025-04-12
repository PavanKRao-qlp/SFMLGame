#include "TrianglePointScene.h"

void TrianglePointScene::Initialize() {}

void TrianglePointScene::OnFixedUpdated() {}

void TrianglePointScene::OnUpdate() {}

Umbra::SharedPtr<Umbra::Scene> TrianglePointScene::InsatiateCopy() {
    return std::make_shared<TrianglePointScene>();
}

void TrianglePointScene::OnBeginPlay() {}

void TrianglePointScene::OnEndPlay() {}
