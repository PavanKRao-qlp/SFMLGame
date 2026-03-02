#pragma once
#include "EnginePCH.h"

#include <chrono>

namespace Umbra {
    class Clock {
    public:
        inline Clock() {
            mStart     = std::chrono::high_resolution_clock::now();
            deltaTime  = 0;
            lastTickTS = 0;
        }

        inline Timestamp GetMS() {
            auto now = std::chrono::high_resolution_clock::now();
            return static_cast<Timestamp>(
                std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart).count());
        }

        inline Timestamp Get() {
            auto now = std::chrono::high_resolution_clock::now();
            return static_cast<Timestamp>(
                std::chrono::duration_cast<std::chrono::microseconds>(now - mStart).count());
        }

        inline void Reset() {
            mStart    = std::chrono::high_resolution_clock::now();
            deltaTime = 0;
        }

        inline void Tick() {
            deltaTime  = ((Get() - lastTickTS) / 1000.f) / 1000.f;
            lastTickTS = Get();
        }

        inline float GetDeltaTime() {
            return deltaTime;
        }

    private:
        float deltaTime;
        Timestamp lastTickTS;
        std::chrono::high_resolution_clock::time_point mStart;
    };

    class EngineTime {
    private:
        inline static Clock clock;
        inline static float mTimeScale = 1.0f;
        inline static bool mbPaused    = false;

    public:
        inline static Timestamp GetTimestampMS() {
            return clock.GetMS();
        }
        inline static Timestamp GetTimestamp() {
            return clock.Get();
        }
        inline static void Reset() {
            clock.Reset();
        }
        inline static float Tick() {
            clock.Tick();
            return GetDeltaTime();
        }
        /// Raw unscaled delta time
        inline static float GetDeltaTime() {
            return clock.GetDeltaTime();
        }
        /// Delta time affected by pause and time scale
        inline static float GetScaledDeltaTime() {
            return mbPaused ? 0.f : clock.GetDeltaTime() * mTimeScale;
        }

        inline static void SetTimeScale(float _timeScale) {
            mTimeScale = _timeScale;
        }
        inline static float GetTimeScale() {
            return mTimeScale;
        }
        inline static void SetPaused(bool _paused) {
            mbPaused = _paused;
        }
        inline static bool IsPaused() {
            return mbPaused;
        }
    };
} // namespace Umbra
