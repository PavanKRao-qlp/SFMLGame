#pragma once
#include <algorithm> // For std::min and std::max
#include <cmath>

namespace Umbra::Math {

    /**
     * @brief Constant value for PI (π).
     */
    const float PI = 3.14159265358979323846f;

    /**
     * @brief Small constant used for floating-point comparison precision.
     */
    const float EPSILON = 1e-8;

    /**
     * @brief Converts radians to degrees.
     * @param _radians Angle in radians.
     * @return Angle in degrees.
     */
    static inline float RadianToDegree(float _radians) {
        return _radians * (180 / PI);
    }

    /**
     * @brief Converts degrees to radians.
     * @param _degrees Angle in degrees.
     * @return Angle in radians.
     */
    static inline float DegreeToRadian(float _degrees) {
        return _degrees * (PI / 180);
    }

    /**
     * @brief Returns the sine of a value (in radians).
     */
    static inline float Sin(float _value) {
        return sin(_value);
    }

    /**
     * @brief Returns the cosine of a value (in radians).
     */
    static inline float Cos(float _value) {
        return cos(_value);
    }

    /**
     * @brief Returns the tangent of a value (in radians).
     */
    static inline float Tan(float _value) {
        return tan(_value);
    }

    /**
     * @brief Returns the arcsine (inverse of sine) of a value.
     * @param _value Input value in range [-1, 1].
     * @return Angle in radians.
     */
    static inline float Asin(float _value) {
        return asin(_value);
    }

    /**
     * @brief Returns the arccosine (inverse of cosine) of a value.
     * @param _value Input value in range [-1, 1].
     * @return Angle in radians.
     */
    static inline float Acos(float _value) {
        return acos(_value);
    }

    /**
     * @brief Returns the arctangent (inverse of tangent) of a value.
     */
    static inline float Atan(float _value) {
        return atan(_value);
    }

    /**
     * @brief Returns the arctangent of y/x using signs to determine the correct quadrant.
     * @param _y Y coordinate.
     * @param _x X coordinate.
     * @return Angle in radians between the positive x-axis and the point (_x, _y).
     */
    static inline float Atan2(float _y, float _x) {
        return atan2(_y, _x);
    }

    /**
     * @brief Returns the square root of a value.
     */
    static inline float Sqrt(float _value) {
        return sqrt(_value);
    }

    /**
     * @brief Returns the value of a base raised to the power of exponent.
     */
    static inline float Pow(float _base, float _exp) {
        return pow(_base, _exp);
    }

    /**
     * @brief Returns the exponential (e^x) of a value.
     */
    static inline float Exp(float _exp) {
        return exp(_exp);
    }

    /**
     * @brief Returns the absolute value of a number.
     */
    static inline float Abs(float _value) {
        return abs(_value);
    }

    /**
     * @brief Returns the natural logarithm (base e) of a value.
     */
    static inline float Log(float _value) {
        return log(_value);
    }

    /**
     * @brief Returns the base-10 logarithm of a value.
     */
    static inline float Log10(float _value) {
        return log10(_value);
    }

    /**
     * @brief Returns the floating-point remainder of dividing two values.
     * @param Value Dividend.
     * @param Modulus Divisor.
     * @return The remainder after division.
     */
    static inline float Fmod(double Value, double Modulus) {
        return fmod(Value, Modulus);
    }

    /**
     * @brief Returns the largest integer less than or equal to the value.
     */
    static inline float Floor(float _value) {
        return floor(_value);
    }

    /**
     * @brief Returns the smallest integer greater than or equal to the value.
     */
    static inline float Ceil(float _value) {
        return ceil(_value);
    }

    /**
     * @brief Returns the nearest integer to the value.
     */
    static inline float Round(float _value) {
        return round(_value);
    }

    /**
     * @brief Returns the smaller of two values.
     */
    static inline float Min(float _a, float _b) {
        return std::min(_a, _b);
    }

    /**
     * @brief Returns the larger of two values.
     */
    static inline float Max(float _a, float _b) {
        return std::max(_a, _b);
    }

    /**
     * @brief Clamps a value between a minimum and maximum range.
     * @param _value The input value.
     * @param _min Minimum allowed value.
     * @param _max Maximum allowed value.
     * @return Clamped value.
     */
    static inline float Clamp(float _value, float _min, float _max) {
        _value = _value < _min ? _min : _value;
        return _value > _max ? _max : _value;
    }

    /**
     * @brief Performs linear interpolation between two values.
     * @param _a Start value.
     * @param _b End value.
     * @param Alpha Interpolation factor in range [0, 1].
     * @return Interpolated value.
     */
    static inline float Lerp(float _a, float _b, float Alpha) {
        return _a + (_b - _a) * Alpha;
    }

    /**
     * @brief Performs linear interpolation with the interpolation factor clamped between 0 and 1.
     * @param _a Start value.
     * @param _b End value.
     * @param Alpha Interpolation factor.
     * @return Clamped interpolated value.
     */
    static inline float LerpClamped(float _a, float _b, float Alpha) {
        return Lerp(_a, _b, Clamp(Alpha, 0.0f, 1.0f));
    }

} // namespace Umbra::Math
