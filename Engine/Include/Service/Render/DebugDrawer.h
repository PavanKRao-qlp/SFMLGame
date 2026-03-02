#pragma once
#include "EnginePCH.h"
#include "Graphics/Color.h"
#include "Math/Bounds.h"
#include "Math/Vector.h"

namespace Umbra {

    class IRenderDevice;

    class DebugDrawer {
    public:
        void DrawLine(const Math::Vector2f& _from, const Math::Vector2f& _to,
            const Color& _color = Color::Green);

        void DrawCircle(const Math::Vector2f& _position, float _radius,
            bool _bFilled = false, const Color& _color = Color::White);

        void DrawBox(const Math::Bounds2D& _bounds,
            bool _bFilled = false, const Color& _color = Color::White);

        void DrawOrientedBox(const Math::Bounds2D& _bounds, float _angle,
            bool _bFilled = false, const Color& _color = Color::White);

        void Flush(IRenderDevice* _renderDevice);

        uint32 GetPrimitiveCount() const;

    private:
        struct DebugLine {
            Math::Vector2f From;
            Math::Vector2f To;
            Color LineColor;
        };

        struct DebugCircleCmd {
            Math::Vector2f Position;
            float Radius;
            bool bFilled;
            Color CircleColor;
        };

        struct DebugBoxCmd {
            Math::Vector2f Center;
            Math::Vector2f Size;
            Math::Vector2f Origin;
            float Angle;
            bool bFilled;
            Color BoxColor;
        };

        Vector<DebugLine> mLineCache;
        Vector<DebugCircleCmd> mCircleCache;
        Vector<DebugBoxCmd> mBoxCache;
    };

} // namespace Umbra
