#pragma once
#include "EnginePCH.h"
#include "mathutil.h"

namespace Umbra::Math {

struct Vector2f{
    public:
        Vector2f();
        Vector2f(float _x, float _y);

        const static Vector2f Zero;
        const static Vector2f One;
        const static Vector2f Up;
        const static Vector2f Down;
        const static Vector2f Left;
        const static Vector2f Right;

        Vector2f operator* (const float _value) const;
        Vector2f operator/ (const float _value) const;
        Vector2f operator+ (const Vector2f& _vector) const;
        Vector2f operator- (const Vector2f& _vector) const;
        bool operator== (const Vector2f& _vector) const;
        void operator/= (const float _value);
        void operator*= (const float _value);
        void operator+= (const Vector2f& _vector);
        void operator-= (const Vector2f& _vector);
        
        static Vector2f Lerp(const Vector2f& _fromVector, const Vector2f& _toVector, float _alpha);
        static float Dot(const Vector2f& _a, const Vector2f& _b,);
        static float Dot(float _magnitudeVectorA, float _magnitudeVectorB, float _angle);

        float Magnitude() const;
        float SqrMagnitude() const;
        Vector2f GetNormalized();

        
        float x = 0;
        float y = 0;
};
} // namespace Umbra::MATH
