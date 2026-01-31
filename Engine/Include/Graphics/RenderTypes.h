#pragma once
#include "EnginePCH.h"
#include "Graphics/Color.h"
#include "Math/Vector.h"

namespace Umbra {

    struct FloatRect {
        float Left   = 0.f;
        float Top    = 0.f;
        float Width  = 0.f;
        float Height = 0.f;

        FloatRect() = default;
        FloatRect(float _left, float _top, float _width, float _height)
            : Left(_left), Top(_top), Width(_width), Height(_height) {}
    };

    struct Vertex {
        Math::Vector2f Position;
        Color VertexColor;
        Math::Vector2f TexCoords;
    };

    enum class EWindowEvent : int {
        None = 0,
        Closed,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        JoystickButtonPressed,
        Resized
    };

} // namespace Umbra
