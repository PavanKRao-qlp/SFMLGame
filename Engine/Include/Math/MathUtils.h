#pragma once
#include <cmath>
#include "EnginePCH.h"

namespace Umbra::Math {

const float PI = 3.14159265358979323846f;

static inline float RadianToDegree(float _radians){
    return _radians * (180/PI);
};
static inline float DegreeToRadain(float _degrees){
    return _degrees * (PI/180);
};

static inline floatSin(float Value)
{
    return sin(Value);
};

static inline float Cos(float Value)
{
    return cos(Value);
};

static inline float Tan(float Value)
{
    return tan(Value);
};

static inline float Asin(float Value)
{
    return asin(Value);
};

static inline float Acos(float Value)
{
    return acos(Value);
};

static inline float Atan(float Value)
{
    return atan(Value);
};

static inline float Atan2(float Y, float X)
{
    return atan2(Y, X);
};

static inline float Sqrt(float Value)
{
    return sqrt(Value);
};

static inline float Pow(float Base, float Exponent)
{
    return pow(Base, Exponent);
};

static inline float Exp(float Exponent)
{
    return exp(Exponent);
};

static inline float Abs(float Value)
{
    return abs(Value);
};

static inline float Log(float Value)
{
    return log(Value);
};

static inline float Log10(float Value)
{
    return log10(Value);
};

static inline float Fmod(float Value, float Modulus)
{
    return fmod(Value, Modulus);
};

static inline float Floor(float Value)
{
    return floor(Value);
};

static inline float Ceil(float Value)
{
    return ceil(Value);
};

static inline float Round(float Value)
{
    return round(Value);
};

static inline float Min(float A, float B)
{
    return std::min(A, B);
};

static inline float Max(float A, float B)
{
    return std::max(A, B);
};

static inline float Clamp(float Value, float Min, float Max)
{
    return (Value < Min) ? Min : (Min < Value) ? Min : Value;
};

static inline float Lerp(float A, float B, float Alpha)
{
    return A + (B - A) * Alpha;
};

static inline float LerpClamped(float A, float B, float Alpha)
{
    return Lerp(A, B, Clamp(Alpha, 0.0f, 1.0f));
};

}