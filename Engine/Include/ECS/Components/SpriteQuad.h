#pragma once
#include "Asset/Texture.h"
#include "ECS/Component.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTypes.h"
#include "Umbra.h"

namespace Umbra {
    struct SpriteComponent : Component {
    public:
        inline SpriteComponent(Color _color = Color::White)
            : color(_color) {}

        inline SpriteComponent(SharedPtr<Texture>& _refTexture, Color _color = Color::White)
            : color(_color), refTexture(_refTexture) {}

        inline SpriteComponent(SharedPtr<Texture>& _refTexture, int32 _zOrder, Color _color = Color::White)
            : color(_color), refTexture(_refTexture), zOrder(_zOrder) {}

        Color color;
        SharedPtr<Texture> refTexture;
        int32 zOrder = 0;
        FloatRect uvRect = {0.f, 0.f, 1.f, 1.f};
    };
} // namespace Umbra
