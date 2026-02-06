#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Graphics/Color.h"
#include "Graphics/IRenderDevice.h"
#include "Math/Bounds.h"
#include "UI/ImGuiBackend.h"
#include <algorithm>

namespace Umbra {

    // Utility function to convert world coordinates to render coordinates
    inline Math::Vector2f WorldToRenderCoords(const Math::Vector2f& _worldPos) {
        return Math::Vector2f(_worldPos.x, -_worldPos.y);
    }

    inline float WorldToRenderAngle(float _worldAngle) {
        return -_worldAngle;
    }

    class RenderSystem : public System {
    public:
        inline RenderSystem(IRenderDevice* _renderDevice, ImGuiBackend* _imGuiBackend)
            : System(std::make_unique<ECView<TransformComponent, SpriteComponent>>()) {
            mRenderDevice = _renderDevice;
            mImguiBackend = _imGuiBackend;
            mSortedEntities.reserve(256);
        }
        inline ~RenderSystem() {}

        inline void Update() override {
            // Get view bounds for frustum culling
            FloatRect viewBounds = mRenderDevice->GetViewBounds();
            float cullMargin = 50.0f; // Extra margin to prevent pop-in

            // Build sorted render list
            mSortedEntities.clear();
            for (EntityID entity : mView->mEntities) {
                const SpriteComponent* sprite = mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);

                // Frustum culling - skip entities outside view
                Math::Vector2f renderPos = WorldToRenderCoords(transform->Position);
                if (!IsInViewBounds(renderPos, transform->Size, viewBounds, cullMargin)) {
                    continue;
                }

                mSortedEntities.push_back({entity, sprite->zOrder});
            }

            // Sort by zOrder (lower values render first, appearing behind)
            std::stable_sort(mSortedEntities.begin(), mSortedEntities.end(),
                [](const SortedEntity& _a, const SortedEntity& _b) {
                    return _a.zOrder < _b.zOrder;
                });

            // Render using batching for better performance
            mRenderDevice->BeginBatch();

            for (const auto& sortedEntity : mSortedEntities) {
                const SpriteComponent* sprite = mView->ecsRegister->GetComponent<SpriteComponent>(sortedEntity.entityId);
                const TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(sortedEntity.entityId);

                Math::Vector2f renderPos = WorldToRenderCoords(transform->Position);
                float renderAngle = WorldToRenderAngle(transform->Angle);

                void* textureHandle = sprite->refTexture ? sprite->refTexture->GetNativeHandle() : nullptr;
                mRenderDevice->BatchQuad(
                    renderPos,
                    transform->Size,
                    transform->Pivot,
                    renderAngle,
                    sprite->color,
                    textureHandle,
                    sprite->uvRect);
            }

            mRenderDevice->EndBatch();

            FlushDebugDraw();

            mRenderDevice->Display();
        }

        static inline void DebugDrawLine(
            const Math::Vector2f& _from, const Math::Vector2f& _to, const Color& _color = Color::Green) {
            DebugLineCache.push_back({_from, _to, _color});
        }

        static inline void DebugDrawCircle(const Math::Vector2f& _position, const float _radius, bool _bFilled = false,
            const Color& _color = Color::White) {
            DebugCircleCache.push_back({_position, _radius, _bFilled, _color});
        }

        static inline void DrawDebugBox(
            const Math::Bounds2D& _bounds, bool _bFilled = false, const Color& _color = Color::White) {
            DebugBoxCache.push_back({_bounds.Center, _bounds.Size,
                Math::Vector2f(0.5f, 0.5f), 0.f, _bFilled, _color});
        }

        static inline void DrawDebugOrientedBox(const Math::Bounds2D& _bounds, float _angle, bool _bFilled = false,
            const Color& _color = Color::White) {
            DebugBoxCache.push_back({_bounds.Center, _bounds.Size,
                Math::Vector2f(0.5f, 0.5f), _angle, _bFilled, _color});
        }

    protected:
        IRenderDevice* mRenderDevice = nullptr;
        ImGuiBackend* mImguiBackend  = nullptr;

    private:
        struct SortedEntity {
            EntityID entityId;
            int32 zOrder;
        };

        Vector<SortedEntity> mSortedEntities;

        inline bool IsInViewBounds(const Math::Vector2f& _renderPos, const Math::Vector2f& _size,
            const FloatRect& _viewBounds, float _margin) const {
            float halfWidth = _size.x * 0.5f + _margin;
            float halfHeight = _size.y * 0.5f + _margin;

            float left = _renderPos.x - halfWidth;
            float right = _renderPos.x + halfWidth;
            float top = _renderPos.y - halfHeight;
            float bottom = _renderPos.y + halfHeight;

            float viewLeft = _viewBounds.Left;
            float viewRight = _viewBounds.Left + _viewBounds.Width;
            float viewTop = _viewBounds.Top;
            float viewBottom = _viewBounds.Top + _viewBounds.Height;

            return !(right < viewLeft || left > viewRight || bottom < viewTop || top > viewBottom);
        }

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

        inline void FlushDebugDraw() {
            for (const auto& line : DebugLineCache) {
                mRenderDevice->DrawLine(
                    WorldToRenderCoords(line.From),
                    WorldToRenderCoords(line.To),
                    line.LineColor);
            }
            for (const auto& circle : DebugCircleCache) {
                mRenderDevice->DrawCircle(
                    WorldToRenderCoords(circle.Position),
                    circle.Radius, circle.bFilled, circle.CircleColor);
            }
            for (const auto& box : DebugBoxCache) {
                mRenderDevice->DrawRect(
                    WorldToRenderCoords(box.Center),
                    box.Size,
                    Math::Vector2f(box.Origin.x * box.Size.x, box.Origin.y * box.Size.y),
                    WorldToRenderAngle(box.Angle),
                    box.bFilled, box.BoxColor);
            }
            DebugLineCache.clear();
            DebugCircleCache.clear();
            DebugBoxCache.clear();
        }

        inline static Vector<DebugLine> DebugLineCache;
        inline static Vector<DebugCircleCmd> DebugCircleCache;
        inline static Vector<DebugBoxCmd> DebugBoxCache;
    };
} // namespace Umbra
