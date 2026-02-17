#include "Service/Render/DebugDrawer.h"
#include "Graphics/IRenderDevice.h"

namespace Umbra {

    static Math::Vector2f WorldToRenderCoords(const Math::Vector2f& _worldPos) {
        return Math::Vector2f(_worldPos.x, -_worldPos.y);
    }

    static float WorldToRenderAngle(float _worldAngle) {
        return -_worldAngle;
    }

    void DebugDrawer::DrawLine(const Math::Vector2f& _from, const Math::Vector2f& _to, const Color& _color) {
        mLineCache.push_back({_from, _to, _color});
    }

    void DebugDrawer::DrawCircle(const Math::Vector2f& _position, float _radius, bool _bFilled,
        const Color& _color) {
        mCircleCache.push_back({_position, _radius, _bFilled, _color});
    }

    void DebugDrawer::DrawBox(const Math::Bounds2D& _bounds, bool _bFilled, const Color& _color) {
        mBoxCache.push_back({_bounds.Center, _bounds.Size,
            Math::Vector2f(0.5f, 0.5f), 0.f, _bFilled, _color});
    }

    void DebugDrawer::DrawOrientedBox(const Math::Bounds2D& _bounds, float _angle, bool _bFilled,
        const Color& _color) {
        mBoxCache.push_back({_bounds.Center, _bounds.Size,
            Math::Vector2f(0.5f, 0.5f), _angle, _bFilled, _color});
    }

    void DebugDrawer::Flush(IRenderDevice* _renderDevice) {
        for (const auto& line : mLineCache) {
            _renderDevice->DrawLine(
                WorldToRenderCoords(line.From),
                WorldToRenderCoords(line.To),
                line.LineColor);
        }
        for (const auto& circle : mCircleCache) {
            _renderDevice->DrawCircle(
                WorldToRenderCoords(circle.Position),
                circle.Radius, circle.bFilled, circle.CircleColor);
        }
        for (const auto& box : mBoxCache) {
            _renderDevice->DrawRect(
                WorldToRenderCoords(box.Center),
                box.Size,
                Math::Vector2f(box.Origin.x * box.Size.x, box.Origin.y * box.Size.y),
                WorldToRenderAngle(box.Angle),
                box.bFilled, box.BoxColor);
        }
        mLineCache.clear();
        mCircleCache.clear();
        mBoxCache.clear();
    }

    uint32 DebugDrawer::GetPrimitiveCount() const {
        return static_cast<uint32>(mLineCache.size() + mCircleCache.size() + mBoxCache.size());
    }

} // namespace Umbra
