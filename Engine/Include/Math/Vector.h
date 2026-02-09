#pragma once
#include "EnginePCH.h"
#include "Math/MathUtils.h"

namespace Umbra::Math {

    /**
     * @brief A generic 2D vector class supporting common vector operations.
     *
     * @tparam T Numeric type (e.g., int, float, double).
     */
    template <typename T>
    struct TVector {
    public:
        /**
         * @brief Constructs a vector with given x and y values.
         */
        TVector(T x = 0, T y = 0) : x(x), y(y) {}

        /**
         * @brief Constructs a vector from another TVector with a different numeric type.
         */
        template <typename U>
        explicit TVector(const TVector<U>& _vector) : x(static_cast<T>(_vector.x)), y(static_cast<T>(_vector.y)) {}

        /**
         * @brief Returns the magnitude (length) of the vector.
         */
        float Magnitude();

        /**
         * @brief Returns the squared magnitude of the vector (avoids sqrt).
         */
        float SquareMagnitude();

        /**
         * @brief Normalizes the vector to unit length (modifies in place).
         */
        void Normalize();

        /**
         * @brief Returns a normalized (unit length) copy of the vector.
         */
        TVector<T> GetNormalized();


        /**
         * @brief Returns a copy of the vector rotated counter-clockwise by a given angle in degrees.
         *
         * @param _Angle Rotation angle in degrees.
         */
        TVector<T> GetRotated(float _angle);


        /**
         * @brief Returns the unsigned angle in degrees between two vectors.
         */
        static float Angle(TVector& _from, TVector& _to);

        /**
         * @brief Returns the signed angle in degrees between two vectors.
         *
         * Positive if counterclockwise, negative if clockwise.
         */
        static float AngleSigned(TVector& _from, TVector& _to);

        /**
         * @brief Returns the dot product of two vectors.
         */
        static float Dot(const TVector& _vectorA, const TVector& _vectorB);

        /**
         * @brief Returns the 2D cross product (scalar) of two vectors.
         *
         * Useful for determining relative orientation (left/right).
         */
        static float Cross2D(const TVector& _vectorA, const TVector& _vectorB);

        /**
         * @brief Returns a vector that is perpendicular (rotated 90° counterclockwise).
         */
        static TVector<T> Perpendicular(TVector& _vectorA);

        // /**
        //  * @brief Returns the perpendicular distance from _vectorA to _vectorB.
        //  *
        //  * This is essentially the magnitude of the cross product.
        //  */
        // static float Perpendicular(TVector& _vectorA, TVector& _vectorB);

        /**
         * @brief Returns the scalar reflection of vector A against vector B.
         */
        static float Reflect(TVector& _vectorA, TVector& _vectorB);

        /**
         * @brief Linearly interpolates between two vectors by t (0 to 1).
         */
        static float Lerp(TVector& _vectorA, TVector& _vectorB, float _t);

        /// X and Y components
        T x;
        T y;

        // Optional common direction constants
        // inline static const TVector<T> Up    = TVector<T>(0, 1);
        // inline static const TVector<T> Down  = TVector<T>(0, -1);
        // inline static const TVector<T> Left  = TVector<T>(-1, 0);
        // inline static const TVector<T> Right = TVector<T>(1, 0);
        // inline static const TVector<T> Zero  = TVector<T>(0, 0);
        // inline static const TVector<T> One   = TVector<T>(1, 1);

    private:
        // You could place helper methods or protected internals here
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
    inline TVector<T> TVector<T>::GetRotated(float _angle) {
        float angleRad = DegreeToRadian(_angle);
        TVector<T> rotated(x, y);
        T xNew    = rotated.x * Cos(angleRad) - rotated.y * Sin(angleRad);
        T yNew    = rotated.x * Sin(angleRad) + rotated.y * Cos(angleRad);
        rotated.x = xNew;
        rotated.y = yNew;
        return rotated;
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
    inline float TVector<T>::Dot(const TVector& _vectorA, const TVector& _vectorB) {
        return (_vectorA.x * _vectorB.x + _vectorA.y * _vectorB.y);
    }

    template <typename T>
    inline float TVector<T>::Cross2D(const TVector& _vectorA, const TVector& _vectorB) {
        return (_vectorA.x * _vectorB.y - _vectorA.y * _vectorB.x);
    }

    template <typename T>
    inline TVector<T> TVector<T>::Perpendicular(TVector& _vectorA) {
        TVector perpendicular(-_vectorA.y, _vectorA.x);
        return perpendicular;
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
