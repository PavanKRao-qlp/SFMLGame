#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Graphics/Color.h"
#include "Graphics/IRenderDevice.h"
#include "Math/Bounds.h"
#include "UI/ImGuiBackend.h"

namespace Umbra {
    class RenderSystem : public System {
    public:
        inline RenderSystem(IRenderDevice* _renderDevice, ImGuiBackend* _imGuiBackend)
            : System(std::make_unique<ECView<TransformComponent, SpriteComponent>>()) {
            mRenderDevice = _renderDevice;
            mImguiBackend = _imGuiBackend;
        }
        inline ~RenderSystem() {}
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                const SpriteComponent* sprite       = mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);

                if (sprite->refTexture != nullptr) {
                    mRenderDevice->DrawTexturedRect(
                        Math::Vector2f((float) transform->Position.x, (float) -transform->Position.y),
                        Math::Vector2f(transform->Size.x, transform->Size.y),
                        Math::Vector2f(transform->Pivot.x, transform->Pivot.y),
                        -transform->Angle,
                        sprite->color,
                        sprite->refTexture->GetNativeHandle());
                } else {
                    mRenderDevice->DrawFilledRect(
                        Math::Vector2f((float) transform->Position.x, (float) -transform->Position.y),
                        Math::Vector2f(transform->Size.x, transform->Size.y),
                        Math::Vector2f(transform->Pivot.x, transform->Pivot.y),
                        -transform->Angle,
                        sprite->color);
                }
            }

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
                    Math::Vector2f(line.From.x, -line.From.y),
                    Math::Vector2f(line.To.x, -line.To.y),
                    line.LineColor);
            }
            for (const auto& circle : DebugCircleCache) {
                mRenderDevice->DrawCircle(
                    Math::Vector2f(circle.Position.x, -circle.Position.y),
                    circle.Radius, circle.bFilled, circle.CircleColor);
            }
            for (const auto& box : DebugBoxCache) {
                mRenderDevice->DrawRect(
                    Math::Vector2f(box.Center.x, -1.f * box.Center.y),
                    box.Size,
                    Math::Vector2f(box.Origin.x * box.Size.x, box.Origin.y * box.Size.y),
                    -box.Angle,
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
