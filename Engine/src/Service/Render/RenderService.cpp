#include "Service/Render/RenderService.h"
#include "Graphics/IRenderDevice.h"
#include <algorithm>

namespace Umbra {

    static Math::Vector2f WorldToRenderCoords(const Math::Vector2f& _worldPos) {
        return Math::Vector2f(_worldPos.x, -_worldPos.y);
    }

    static float WorldToRenderAngle(float _worldAngle) {
        return -_worldAngle;
    }

    void RenderService::Initialize(IRenderDevice* _renderDevice) {
        mRenderDevice = _renderDevice;
        mQuadQueue.reserve(256);
    }

    void RenderService::Shutdown() {
        mRenderDevice = nullptr;
        mQuadQueue.clear();
    }

    void RenderService::BeginFrame() {
        mQuadQueue.clear();
        mStats = {};
        if (mRenderDevice) {
            mRenderDevice->Clear(Color::Black);
        }
    }

    void RenderService::SubmitQuad(const RenderQuad& _quad) {
        mQuadQueue.push_back(_quad);
        mStats.QuadsSubmitted++;
    }

    void RenderService::Flush() {
        if (!mRenderDevice) {
            return;
        }

        FloatRect vb     = mRenderDevice->GetViewBounds();
        Math::Bounds2D viewBounds(
            {vb.Left + vb.Width * 0.5f, vb.Top + vb.Height * 0.5f},
            {vb.Width, vb.Height});
        float cullMargin = 50.0f;

        // Sort by ZOrder (lower values render first, appearing behind)
        std::stable_sort(mQuadQueue.begin(), mQuadQueue.end(),
            [](const RenderQuad& _a, const RenderQuad& _b) {
                return _a.ZOrder < _b.ZOrder;
            });

        mRenderDevice->BeginBatch();

        for (const auto& quad : mQuadQueue) {
            Math::Vector2f renderPos = WorldToRenderCoords(quad.Position);

            if (!IsInViewBounds(renderPos, quad.Size, viewBounds, cullMargin)) {
                mStats.QuadsCulled++;
                continue;
            }

            float renderAngle   = WorldToRenderAngle(quad.Angle);
            void* textureHandle = quad.RefTexture ? quad.RefTexture->GetNativeHandle() : nullptr;
            void* shaderHandle  = quad.RefShader  ? quad.RefShader->GetNativeHandle()  : nullptr;

            mRenderDevice->BatchQuad(
                renderPos, quad.Size, quad.Pivot,
                renderAngle, quad.Tint, textureHandle, quad.UVRect, shaderHandle);

            mStats.QuadsRendered++;
        }

        mRenderDevice->EndBatch();

        // Flush debug primitives
        mStats.DebugPrimitives = mDebugDrawer.GetPrimitiveCount();
        mDebugDrawer.Flush(mRenderDevice);
    }

    // --- Debug Draw API ---

    void RenderService::DebugDrawLine(const Math::Vector2f& _from, const Math::Vector2f& _to,
        const Color& _color) {
        mDebugDrawer.DrawLine(_from, _to, _color);
    }

    void RenderService::DebugDrawCircle(const Math::Vector2f& _position, float _radius,
        bool _bFilled, const Color& _color) {
        mDebugDrawer.DrawCircle(_position, _radius, _bFilled, _color);
    }

    void RenderService::DebugDrawBox(const Math::Bounds2D& _bounds,
        bool _bFilled, const Color& _color) {
        mDebugDrawer.DrawBox(_bounds, _bFilled, _color);
    }

    void RenderService::DebugDrawOrientedBox(const Math::Bounds2D& _bounds, float _angle,
        bool _bFilled, const Color& _color) {
        mDebugDrawer.DrawOrientedBox(_bounds, _angle, _bFilled, _color);
    }

    // --- Camera State ---

    void RenderService::SetCamera(const Math::Vector2f& _center, float _orthoSize, float _renderAspect) {
        mCameraCenter = _center;
        mOrthoSize    = _orthoSize;
        mRenderAspect = _renderAspect;
    }

    FloatRect RenderService::GetViewBounds() const {
        float halfWidth  = mOrthoSize * mRenderAspect;
        float halfHeight = mOrthoSize;
        return FloatRect(
            mCameraCenter.x - halfWidth,
            mCameraCenter.y - halfHeight,
            halfWidth * 2.f,
            halfHeight * 2.f);
    }

    // --- Private ---

    bool RenderService::IsInViewBounds(const Math::Vector2f& _renderPos, const Math::Vector2f& _size,
        const Math::Bounds2D& _viewBounds, float _margin) const {
        Math::Vector2f marginVec(_margin * 2.f, _margin * 2.f);
        Math::Bounds2D quadBounds(_renderPos, _size + marginVec);
        return quadBounds.Intersects(_viewBounds);
    }

} // namespace Umbra
