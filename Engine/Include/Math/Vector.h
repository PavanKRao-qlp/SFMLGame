#pragma once
#include "EnginePCH.h"
#include "Math/MathUtils.h"
#include "hlsl++.h"

namespace Umbra::Math {
    // using namespace hlslpp;

    template <typename T>
    struct TVector {
    public:
        TVector(T x = 0, T y = 0) : x(x), y(y) {}

        template <typename U>
        explicit TVector(const TVector<U>& _vector) : x(static_cast<T>(_vector.x)), y(static_cast<T>(_vector.y)){};

        float Magnitude();
        float SquareMagnitude();
        void Normalize();
        TVector<T> GetNormalized();

        static float Angle(TVector& _from, TVector& _to);
        static float AngleSigned(TVector& _from, TVector& _to);
        static float Dot(TVector& _vectorA, TVector& _vectorB);
        // static float GetProjection(TVector& _vectorA, TVector& _vectorB);
        static float Cross2D(TVector& _vectorA, TVector& _vectorB);
        static float Perpendicular(TVector& _vectorA, TVector& _vectorB);
        static float Reflect(TVector& _vectorA, TVector& _vectorB);
        static float Lerp(TVector& _vectorA, TVector& _vectorB, float _t);

        T x;
        T y;
        // inline static const TVector<T> Up    = TVector<T>(0, 1);
        // inline static const TVector<T> Down  = TVector<T>(0, -1);
        // inline static const TVector<T> Left  = TVector<T>(-1, 0);
        // inline static const TVector<T> Right = TVector<T>(1, 0);
        // inline static const TVector<T> Zero  = TVector<T>(0, 0);
        // inline static const TVector<T> One   = TVector<T>(1, 1);

    private:
    };

    using FVector2D   = TVector<float>;
    using IntVector2D = TVector<int>;
    using Vector2lf   = TVector<double>;
    using Vector2f    = TVector<float>;
    using Vector2i    = TVector<int>;


    template <typename T>
    inline TVector<T> operator+(const TVector<T>& _vectorA, const TVector<T>& _vectorB) {
        return TVector<T>(_vectorA.x + _vectorB.x, _vectorA.y + _vectorB.y);
    }

    template <typename T>
    inline TVector<T> operator-(const TVector<T>& _vectorA, const TVector<T>& _vectorB) {
        return TVector<T>(_vectorA.x - _vectorB.x, _vectorA.y - _vectorB.y);
    }

    template <typename T>
    inline TVector<T> operator+=(TVector<T>& _vectorA, const TVector<T>& _vectorB) {
        _vectorA.x += _vectorB.x;
        _vectorA.y += _vectorB.y;
        return _vectorA;
    }

    template <typename T>
    inline TVector<T> operator-=(TVector<T>& _vectorA, const TVector<T>& _vectorB) {
        _vectorA.x -= _vectorB.x;
        _vectorA.y -= _vectorB.y;
        return _vectorA;
    }

    template <typename T>
    inline TVector<T> operator*(const TVector<T>& _vector, T _value) {
        return TVector<T>(_vector.x * _value, _vector.y * _value);
    }

    template <typename T, typename U>
    inline TVector<T> operator*(const TVector<T>& _vector, U _value) {
        return TVector<T>(static_cast<T>(_vector.x * _value), static_cast<T>(_vector.y * _value));
    }

    template <typename T, typename U>
    inline TVector<T> operator*(U _value, const TVector<T>& _vector) {

        return _vector * _value;
    }

    template <typename T, typename U>
    inline TVector<T> operator/(const TVector<T>& _vector, U _value) {
        return TVector<T>(static_cast<T>(_vector.x / _value), static_cast<T>(_vector.y / _value));
    }

    template <typename T, typename U>
    inline TVector<T> operator*=(TVector<T>& _vector, U _value) {
        _vector.x *= static_cast<T>(_value);
        _vector.y *= static_cast<T>(_value);
        return _vector;
    }

    template <typename T, typename U>
    inline TVector<T> operator/=(TVector<T>& _vector, U _value) {
        _vector.x /= static_cast<T>(_value);
        _vector.y /= static_cast<T>(_value);
        return _vector;
    }

    template <typename T>
    inline bool operator==(TVector<T>& _vectorA, const TVector<T>& _vectorB) {
        return _vectorA.x == _vectorB.x && _vectorA.y == _vectorB.y;
    }

    template <typename T>
    inline bool operator!=(TVector<T>& _vectorA, const TVector<T>& _vectorB) {
        return _vectorA.x != _vectorB.x || _vectorA.y != _vectorB.y;
    }

    template <typename T>
    inline float TVector<T>::Magnitude() {
        return Sqrt((x * x) + (y * y));
    }

    template <typename T>
    inline float TVector<T>::SquareMagnitude() {
        return (x * x) + (y * y);
    }

    template <typename T>
    inline void TVector<T>::Normalize() {
        *this /= Magnitude();
    }

    template <typename T>
    inline TVector<T> TVector<T>::GetNormalized() {
        TVector normalized(x, y);
        normalized.Normalize();
        return normalized;
    }

    template <typename T>
    inline float TVector<T>::Angle(TVector& _from, TVector& _to) {
        return acos(Dot(_from, _to) / (_from.Magnitude * _to.Magnitude));
    }

    template <typename T>
    inline float TVector<T>::AngleSigned(TVector& _from, TVector& _to) {
        return 0.0f;
    }

    template <typename T>
    inline float TVector<T>::Dot(TVector& _vectorA, TVector& _vectorB) {
        return (_vectorA.x * _vectorB);
    }

    template <typename T>
    inline float TVector<T>::Cross2D(TVector& _vectorA, TVector& _vectorB) {
        return 0.0f;
    }

    template <typename T>
    inline float TVector<T>::Perpendicular(TVector& _vectorA, TVector& _vectorB) {
        return 0.0f;
    }

    template <typename T>
    inline float TVector<T>::Reflect(TVector& _vectorA, TVector& _vectorB) {
        return 0.0f;
    }

    template <typename T>
    inline float TVector<T>::Lerp(TVector& _vectorA, TVector& _vectorB, float _t) {
        return 0.0f;
    }
} // namespace Umbra::Math
