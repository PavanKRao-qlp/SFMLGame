#pragma once
#include "Asset/ShaderResource.h"
#include "ECS/Component.h"
#include "Umbra.h"

namespace Umbra {

    // -------------------------------------------------------------------------
    // MaterialComponent
    //
    // Optional component. Attach to any entity that also has a SpriteComponent
    // to apply a shader when that sprite is rendered.
    //
    // Usage:
    //   auto shader = AssetManager::Get().GetShader("Assets/Shaders/grayscale.frag");
    //   world->AddComponent<MaterialComponent>(entity, shader);
    // -------------------------------------------------------------------------
    struct MaterialComponent : Component {
        MaterialComponent() = default;
        explicit MaterialComponent(SharedPtr<Shader> _shader) : shader(std::move(_shader)) {}

        SharedPtr<Shader> shader;
    };

} // namespace Umbra
