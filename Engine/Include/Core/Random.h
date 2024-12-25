#pragma once
#include "EnginePCH.h"
#include "Math/PCGRandomNumberGenerator.h"

namespace Umbra {
    class Random {
    public:
        /* * Set the seed for the RNG]*/
        static inline void SetSeed(uint64 _state, uint64 _sequence = 0xda3e39cb94b95bdbULL) {
            mRandom.SetSeed(_state, _sequence);
        }
        /* * Generate a random float in the range [0, 1]*/
        static inline float GetRandom() {
            return mRandom.GetRandom();
        }
        /* * Generate a random int in the range [Min-Max] inclusive*/
        static inline int RandomRange(int _min, int _max) {
            return mRandom.RandomRange(_min, _max);
        }
        /* * Generate a random float in the range [Min-Max] inclusive*/
        static inline float RandomRange(float _min, float _max) {
            return mRandom.RandomRange(_min, _max);
        }

    private:
        static inline PCGRandomNumberGenerator mRandom;
    };
} // namespace Umbra
