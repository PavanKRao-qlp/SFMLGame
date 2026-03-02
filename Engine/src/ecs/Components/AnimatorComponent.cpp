#include "ECS/Components/AnimatorComponent.h"

namespace Umbra {

    AnimationClip AnimationClip::FromGrid(const String& _name,
                                          Math::Vector2i _frameSize,
                                          int32 _startFrame,
                                          int32 _frameCount,
                                          int32 _framesPerRow,
                                          float _frameDuration,
                                          bool _bLoop) {
        AnimationClip clip;
        clip.Name          = _name;
        clip.FrameDuration = _frameDuration;
        clip.bLoop         = _bLoop;
        clip.Frames.reserve(static_cast<size_t>(_frameCount));

        for (int32 i = 0; i < _frameCount; ++i) {
            int32 index = _startFrame + i;
            int32 col   = index % _framesPerRow;
            int32 row   = index / _framesPerRow;

            AnimationFrame frame;
            frame.Origin = Math::Vector2i(col * _frameSize.x, row * _frameSize.y);
            frame.Size   = _frameSize;
            clip.Frames.push_back(frame);
        }

        return clip;
    }

    void AnimatorComponent::AddClip(AnimationClip _clip) {
        String name  = _clip.Name;
        Clips[name]  = std::move(_clip);
    }

    void AnimatorComponent::Play(const String& _clipName, bool _bReset) {
        if (CurrentClip != _clipName || _bReset) {
            CurrentClip = _clipName;
            if (_bReset) {
                CurrentFrame = 0;
                ElapsedTime  = 0.f;
            }
        }
        bPlaying  = true;
        bFinished = false;
    }

    void AnimatorComponent::Stop() {
        bPlaying     = false;
        CurrentFrame = 0;
        ElapsedTime  = 0.f;
    }

    void AnimatorComponent::Pause() {
        bPlaying = false;
    }

    void AnimatorComponent::Resume() {
        if (!bFinished) {
            bPlaying = true;
        }
    }

    const AnimationClip* AnimatorComponent::GetCurrentClip() const {
        if (CurrentClip.empty()) {
            return nullptr;
        }
        auto it = Clips.find(CurrentClip);
        return it != Clips.end() ? &it->second : nullptr;
    }

} // namespace Umbra
