#pragma once
#include "EnginePCH.h"

namespace Umbra {
    /// @brief Shape types supported by the physics engine
    enum class EShapeType : uint8 {
        None,
        Box, // Axis-Aligned Bounding Box defined by min/max corners
        Circle // Circle defined by center + radius
    };

    class CircleShape {
    public:
        CircleShape() = default;
        explicit CircleShape(float _radius) : mRadius(_radius) {}

        float GetRadius() const {
            return mRadius;
        }
        void SetRadius(float _radius) {
            mRadius = _radius;
        }

    private:
        float mRadius = 0;
    };

    class BoxShape {
    public:
        BoxShape() = default;
        BoxShape(float _width, float _height) {
            mSize = Math::Vector2f(_width, _height);
        }
        BoxShape(Math::Vector2f _size) : mSize(_size) {}

        float GetWidth() const {
            return mSize.x;
        }
        float GetHeight() const {
            return mSize.y;
        }
        void SetWidth(float _width) {
            mSize.x = _width;
        }
        void SetHeight(float _height) {
            mSize.y = _height;
        }

        Math::Vector2f GetSize() const {
            return mSize;
        }

    private:
        Math::Vector2f mSize;
    };

    class ShapeData {
        using ShapeDef = Variant<CircleShape, BoxShape>;

    public:
        ShapeData() : mShapeType(EShapeType::None) {}

        static ShapeData MakeCircle(float _radius) {
            ShapeData data;
            data.mShapeType = EShapeType::Circle;
            data.mShapeData = CircleShape(_radius);
            return data;
        }

        static ShapeData MakeBox(float _width, float _height) {
            ShapeData data;
            data.mShapeType = EShapeType::Box;
            data.mShapeData = BoxShape(_width, _height);
            return data;
        }

        static ShapeData MakeBox(Math::Vector2f _size) {
            ShapeData data;
            data.mShapeType = EShapeType::Box;
            data.mShapeData = BoxShape(_size);
            return data;
        }

        EShapeType GetShapeType() const {
            return mShapeType;
        }

        const CircleShape& GetCircle() const {
            return std::get<CircleShape>(mShapeData);
        }
        CircleShape& GetCircle() {
            return std::get<CircleShape>(mShapeData);
        }

        const BoxShape& GetBox() const {
            return std::get<BoxShape>(mShapeData);
        }
        BoxShape& GetBox() {
            return std::get<BoxShape>(mShapeData);
        }

        bool IsCircle() const {
            return mShapeType == EShapeType::Circle;
        }
        bool IsBox() const {
            return mShapeType == EShapeType::Box;
        }
        bool IsNone() const {
            return mShapeType == EShapeType::None;
        }

    private:
        EShapeType mShapeType;
        ShapeDef mShapeData;
    };
} // namespace Umbra
