#pragma once
#include "EnginePCH.h"
#include "Asset/Texture.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTypes.h"
#include "Math/Vector.h"

namespace Umbra {

    struct RenderQuad {
        Math::Vector2f Position;
        Math::Vector2f Size;
        Math::Vector2f Pivot = {0.5f, 0.5f};
        float Angle          = 0.f;
        int32 ZOrder         = 0;
        Color Tint           = Color::White;
        SharedPtr<Texture> RefTexture;
        FloatRect UVRect = {0.f, 0.f, 1.f, 1.f};
    };

    struct RenderStats {
        uint32 QuadsSubmitted = 0;
        uint32 QuadsRendered  = 0;
        uint32 QuadsCulled    = 0;
        uint32 DebugPrimitives = 0;
    };

} // namespace Umbra
