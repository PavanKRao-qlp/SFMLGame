#pragma once
#include "EnginePCH.h"
#include "Service/Render/DebugDrawer.h"
#include "Service/Render/RenderTypes.h"

namespace Umbra {

    class IRenderDevice;

    class RenderService {
    public:
        RenderService() = default;
        ~RenderService() = default;

        void Initialize(IRenderDevice* _renderDevice);
        void Shutdown();

        // --- Frame Lifecycle ---
        void BeginFrame();
        void SubmitQuad(const RenderQuad& _quad);
        void Flush();

        // --- Debug Draw API ---
        void DebugDrawLine(const Math::Vector2f& _from, const Math::Vector2f& _to,
            const Color& _color = Color::Green);

        void DebugDrawCircle(const Math::Vector2f& _position, float _radius,
            bool _bFilled = false, const Color& _color = Color::White);

        void DebugDrawBox(const Math::Bounds2D& _bounds,
            bool _bFilled = false, const Color& _color = Color::White);

        void DebugDrawOrientedBox(const Math::Bounds2D& _bounds, float _angle,
            bool _bFilled = false, const Color& _color = Color::White);

        // --- Camera State ---
        void SetCamera(const Math::Vector2f& _center, float _orthoSize, float _renderAspect);
        FloatRect GetViewBounds() const;

        // --- Stats ---
        const RenderStats& GetStats() const { return mStats; }

    private:
        bool IsInViewBounds(const Math::Vector2f& _renderPos, const Math::Vector2f& _size,
            const FloatRect& _viewBounds, float _margin) const;

        IRenderDevice* mRenderDevice = nullptr;
        DebugDrawer mDebugDrawer;
        Vector<RenderQuad> mQuadQueue;
        RenderStats mStats;

        // Camera state for frustum culling
        Math::Vector2f mCameraCenter;
        float mOrthoSize    = 75.f;
        float mRenderAspect = 1.f;
    };

} // namespace Umbra
