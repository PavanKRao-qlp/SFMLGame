#pragma once
#include "EnginePCH.h"

#include <SFML/System/Clock.hpp>
namespace Umbra {
    class Clock {
    public:
        inline Timestamp GetMS() {
            return clock.getElapsedTime().asMilliseconds();
        }
        inline Timestamp Get() {
            return clock.getElapsedTime().asMicroseconds();
        }
        inline void Reset() {
            clock.restart();
            deltaTime = 0;
        }
        inline void Tick() {
            deltaTime  = (GetMS() - lastTickTS) / 1000.f;
            lastTickTS = GetMS();
        }
        inline float GetDeltaTime() {
            return deltaTime;
        }

    private:
        float deltaTime;
        Timestamp lastTickTS;
        sf::Clock clock;
    };

    class EngineTime {
    private:
        inline static Clock clock;

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
        inline static float GetDeltaTime() {
            return clock.GetDeltaTime();
        }
    };
} // namespace Umbra
