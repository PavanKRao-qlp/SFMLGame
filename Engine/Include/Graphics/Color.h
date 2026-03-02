#pragma once
#include "EnginePCH.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

namespace Umbra {

    class Color {
    private:
        glm::u8vec4 value; // r, g, b, a as uint8

        static constexpr uint8_t toUint8(float val) {
            return static_cast<uint8_t>(val * 255.0f + 0.5f);
        }

        static constexpr float toFloat(uint8_t val) {
            return static_cast<float>(val) / 255.0f;
        }

    public:
        // Constructors
        Color() : value(255, 255, 255, 255) {}

        Color(float r, float g, float b, float a = 1.0f) : value(toUint8(r), toUint8(g), toUint8(b), toUint8(a)) {}

        explicit Color(const glm::vec4& vec) : value(toUint8(vec.r), toUint8(vec.g), toUint8(vec.b), toUint8(vec.a)) {}

        explicit Color(const glm::u8vec4& vec) : value(vec) {}

        // Accessors - float
        float r() const {
            return toFloat(value.r);
        }
        float g() const {
            return toFloat(value.g);
        }
        float b() const {
            return toFloat(value.b);
        }
        float a() const {
            return toFloat(value.a);
        }

        // Accessors - uint8
        uint8_t r8() const {
            return value.r;
        }
        uint8_t g8() const {
            return value.g;
        }
        uint8_t b8() const {
            return value.b;
        }
        uint8_t a8() const {
            return value.a;
        }

        // Setters (float)
        void setR(float r) {
            value.r = toUint8(r);
        }
        void setG(float g) {
            value.g = toUint8(g);
        }
        void setB(float b) {
            value.b = toUint8(b);
        }
        void setA(float a) {
            value.a = toUint8(a);
        }

        // Setters (uint8)
        void setR8(uint8_t r) {
            value.r = r;
        }
        void setG8(uint8_t g) {
            value.g = g;
        }
        void setB8(uint8_t b) {
            value.b = b;
        }
        void setA8(uint8_t a) {
            value.a = a;
        }

        // Conversion
        glm::vec4 toVec4() const {
            return glm::vec4(r(), g(), b(), a());
        }

        glm::u8vec4 toU8Vec4() const {
            return value;
        }

        static Color fromVec4(const glm::vec4& vec) {
            return Color(vec);
        }

        static Color fromU8Vec4(const glm::u8vec4& vec) {
            return Color(vec);
        }

        static Color FromU8(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255) {
            return Color(glm::u8vec4(_r, _g, _b, _a));
        }

        // Utility
        Color withAlpha(float alpha) const {
            return Color(r(), g(), b(), alpha);
        }

        // Operator overloads
        bool operator==(const Color& other) const {
            return value == other.value;
        }

        bool operator!=(const Color& other) const {
            return !(*this == other);
        }

        inline uint32 Color::ToInteger() const {
            return static_cast<uint32>((value.r << 24) | (value.g << 16) | (value.b << 8) | value.a);
        }
        // Predefined Colors
        static const Color White;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Magenta;
        static const Color Cyan;
        static const Color Transparent;
    };


} // namespace Umbra
