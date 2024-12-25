#pragma once
#include <cmath>

namespace Umbra::Math {

    const float PI = 3.14159265358979323846f;

    static inline float RadianToDegree(float _radians) {
        return _radians * (180 / PI);
    }

    static inline float DegreeToRadian(float _degrees) {
        return _degrees * (PI / 180);
    }

    static inline float Sin(float _value) {
        return sin(_value);
    }

    static inline float Cos(float _value) {
        return cos(_value);
    }

    static inline float Tan(float _value) {
        return tan(_value);
    }

    static inline float Asin(float _value) {
        return asin(_value);
    }

    static inline float Acos(float _value) {
        return acos(_value);
    }

    static inline float Atan(float _value) {
        return atan(_value);
    }

    static inline float Atan2(float _y, float _x) {
        return atan2(_y, _x);
    }

    static inline float Sqrt(float _value) {
        return sqrt(_value);
    }

    static inline float Pow(float _base, float _exp) {
        return pow(_base, _exp);
    }

    static inline float Exp(float _exp) {
        return exp(_exp);
    }

    static inline float Abs(float _value) {
        return abs(_value);
    }

    static inline float Log(float _value) {
        return log(_value);
    }

    static inline float Log10(float _value) {
        return log10(_value);
    }

    static inline float Fmod(float Value, float Modulus) {
        return fmod(Value, Modulus);
    }

    static inline float Floor(float _value) {
        return floor(_value);
    }

    static inline float Ceil(float _value) {
        return ceil(_value);
    }

    static inline float Round(float _value) {
        return round(_value);
    }

    static inline float Min(float _a, float _b) {
        return std::min(_a, _b);
    }

    static inline float Max(float _a, float _b) {
        return std::max(_a, _b);
    }

    static inline float Clamp(float _value, float _min, float _max) {
        _value = _value < _min ? _min : _value;
        return _value > _max ? _max : _value;
    }

    static inline float Lerp(float _a, float _b, float Alpha) {
        return _a + (_b - _a) * Alpha;
    }

    static inline float LerpClamped(float _a, float _b, float Alpha) {
        return Lerp(_a, _b, Clamp(Alpha, 0.0f, 1.0f));
    }

} // namespace Umbra::Math
