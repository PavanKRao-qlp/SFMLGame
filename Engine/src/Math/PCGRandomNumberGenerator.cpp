#include "Math/PCGRandomNumberGenerator.h"
namespace Umbra {
    PCGRandomNumberGenerator::PCGRandomNumberGenerator(uint64 _initState, uint64 _initSequence) {
        SetSeed(_initState, _initSequence);
    }

    PCGRandomNumberGenerator::~PCGRandomNumberGenerator() {}

    void PCGRandomNumberGenerator::SetSeed(uint64 _state, uint64 _sequence) {
        mSequence = _sequence | 1;
        mState    = 0;
        Step(); // Warm-up the generator
        mState += _state;
        Step();
    }

    uint64 PCGRandomNumberGenerator::Step() {
        mState = mState * 6364136223846793005ULL + (mSequence | 1); // Update state using LCG formula
        return mState;
    }

    uint32 PCGRandomNumberGenerator::Output(uint64) {
        uint32 xorShifted = static_cast<uint32>(((mState >> 18u) ^ mState) >> 27u);
        uint32 rot        = static_cast<uint32>(mState >> 59u);
        return (xorShifted >> rot) | (xorShifted << ((-rot) & 31));
    }

    uint32 PCGRandomNumberGenerator::Next() {
        return Output(Step());
    }

    float PCGRandomNumberGenerator::GetRandom() {
        float val = static_cast<float>(Next()) / std::numeric_limits<uint32>::max();
        return val;
    }

    int PCGRandomNumberGenerator::RandomRange(int _min, int _max) {
        return _min + (Next() % (_max - _min + 1));
    }

    float PCGRandomNumberGenerator::RandomRange(float _min, float _max) {
        return _min + (_max - _min) * GetRandom();
    }
} // namespace Umbra
