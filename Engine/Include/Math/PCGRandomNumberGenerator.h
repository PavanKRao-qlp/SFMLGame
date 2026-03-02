#pragma once
#include "EnginePCH.h"
namespace Umbra {
    class PCGRandomNumberGenerator {
    public:
        PCGRandomNumberGenerator(
            uint64 _initState = 0x853c49e6748fea9bULL, uint64 _initSequence = 0xda3e39cb94b95bdbULL);
        ~PCGRandomNumberGenerator();

        /* * Set the seed for the RNG]*/
        void SetSeed(uint64 _state, uint64 _sequence = 0xda3e39cb94b95bdbULL);
        /* * Generate a random float in the range [0, 1]*/
        float GetRandom();
        /* * Generate a random int in the range [Min-Max] inclusive*/
        int RandomRange(int _min, int _max);
        /* * Generate a random float in the range [Min-Max] inclusive*/
        float RandomRange(float _min, float _max);

    private:
        uint64 mState; // Current state of the generator
        uint64 mSequence; // Sequence selector (must be odd)
        uint64 Step(); // Internal step function for updating the state
        uint32 Output(uint64); // Output function to permute bits of the state
        uint32 Next(); // Generate the next random 32-bit integer
    };


} // namespace Umbra
