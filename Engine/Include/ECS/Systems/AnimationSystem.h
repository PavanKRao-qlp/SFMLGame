#pragma once
#include "Core/Clock.h"
#include "ECS/Components/AnimatorComponent.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/System.h"
#include "ECS/View.h"

namespace Umbra {

    // Drives sprite sheet animation by updating SpriteComponent::uvRect each
    // frame based on the active AnimationClip in AnimatorComponent.
    // Run in ESystemPhase::Simulation so uvRect is ready before RenderSyncSystem.
    class AnimationSystem : public System {
    public:
        inline AnimationSystem()
            : System(std::make_unique<ECView<AnimatorComponent, SpriteComponent>>()) {}

        inline void Update() override {
            const float dt = EngineTime::GetDeltaTime();

            for (EntityID entity : mView->mEntities) {
                AnimatorComponent* animator =
                    mView->ecsRegister->GetComponent<AnimatorComponent>(entity);
                SpriteComponent* sprite =
                    mView->ecsRegister->GetComponent<SpriteComponent>(entity);

                if (!animator->bPlaying || animator->CurrentClip.empty()) {
                    continue;
                }

                auto it = animator->Clips.find(animator->CurrentClip);
                if (it == animator->Clips.end()) {
                    continue;
                }

                AnimationClip& clip = it->second;
                if (clip.Frames.empty()) {
                    continue;
                }

                // Advance the frame timer
                float frameDuration = GetFrameDuration(clip, animator->CurrentFrame);
                animator->ElapsedTime += dt * animator->Speed;

                while (animator->ElapsedTime >= frameDuration) {
                    animator->ElapsedTime -= frameDuration;
                    animator->CurrentFrame++;

                    if (animator->CurrentFrame >= static_cast<int32>(clip.Frames.size())) {
                        if (clip.bLoop) {
                            animator->CurrentFrame = 0;
                        } else {
                            animator->CurrentFrame =
                                static_cast<int32>(clip.Frames.size()) - 1;
                            animator->bPlaying  = false;
                            animator->bFinished = true;
                            break;
                        }
                    }
                    frameDuration = GetFrameDuration(clip, animator->CurrentFrame);
                }

                // Map the current frame's pixel rect to normalized UV coords
                if (sprite->refTexture) {
                    Math::Vector2i texSize = sprite->refTexture->GetSize();
                    if (texSize.x > 0 && texSize.y > 0) {
                        const AnimationFrame& frame = clip.Frames[animator->CurrentFrame];
                        sprite->uvRect.Left   = static_cast<float>(frame.Origin.x) / texSize.x;
                        sprite->uvRect.Top    = static_cast<float>(frame.Origin.y) / texSize.y;
                        sprite->uvRect.Width  = static_cast<float>(frame.Size.x)   / texSize.x;
                        sprite->uvRect.Height = static_cast<float>(frame.Size.y)   / texSize.y;
                    }
                }
            }
        }

    private:
        inline float GetFrameDuration(const AnimationClip& _clip, int32 _frameIndex) const {
            float dur = _clip.Frames[_frameIndex].Duration;
            return dur > 0.f ? dur : _clip.FrameDuration;
        }
    };

} // namespace Umbra
