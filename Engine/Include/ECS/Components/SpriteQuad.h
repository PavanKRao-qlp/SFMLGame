#pragma once
#include "Asset/Texture.h"
#include "ECS/Component.h"
#include "Graphics/Color.h"
#include "Umbra.h"

namespace Umbra {
    struct SpriteComponent : Component {
    public:
        inline SpriteComponent(Color _color = Color::White) {
            color = _color;
        }
        inline SpriteComponent(SharedPtr<Texture>& _refTexture, Color _color = Color::White)
            : color(_color), refTexture(_refTexture) {}
        Color color;
        SharedPtr<Texture> refTexture;
    };
} // namespace Umbra
