#pragma once
#include "ECS/Component.h"
#include "EnginePCH.h"
#include "Math/Vector.h"

namespace Umbra {

    struct AnimationFrame {
        Math::Vector2i Origin;  // Top-left pixel on the sprite sheet
        Math::Vector2i Size;    // Frame dimensions in pixels
        float Duration = -1.f; // Per-frame override; -1 uses AnimationClip::FrameDuration
    };

    struct AnimationClip {
        String Name;
        Vector<AnimationFrame> Frames;
        float FrameDuration = 0.1f; // Default seconds per frame
        bool bLoop = true;

        // Build a clip from a uniform-grid sprite sheet.
        // _startFrame is 0-based row-major index into the grid.
        static AnimationClip FromGrid(const String& _name,
                                      Math::Vector2i _frameSize,
                                      int32 _startFrame,
                                      int32 _frameCount,
                                      int32 _framesPerRow,
                                      float _frameDuration = 0.1f,
                                      bool _bLoop         = true);
    };

    struct AnimatorComponent : Component {
        UMap<String, AnimationClip> Clips;
        String CurrentClip;
        int32 CurrentFrame = 0;
        float ElapsedTime  = 0.f;
        float Speed        = 1.f;
        bool bPlaying      = false;
        bool bFinished     = false;

        void AddClip(AnimationClip _clip);

        // Switch to a clip. If _bReset is true the frame counter resets even
        // when switching to the clip that is already playing.
        void Play(const String& _clipName, bool _bReset = true);
        void Stop();
        void Pause();
        void Resume();

        const AnimationClip* GetCurrentClip() const;
    };

} // namespace Umbra
