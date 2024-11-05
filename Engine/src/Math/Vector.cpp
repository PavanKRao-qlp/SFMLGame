#include "Math/Vector.h"

namespace Umbra::Math {

const Vector2f Vector2f::Zero = Vector2f(0.0f, 0.0f);
const Vector2f Vector2f::One = Vector2f(1.0f, 1.0f);
const Vector2f Vector2f::Up = Vector2f(0.0f, 1.0f);
const Vector2f Vector2f::Down = Vector2f(0.0f, -1.0f);
const Vector2f Vector2f::Left = Vector2f(-1.0f, 0.0f);
const Vector2f Vector2f::Right = Vector2f(1.0f, 0.0f);

Vector2f::Vector2f() {
    
}

Vector2f::Vector2f(float _x, float _y):  x(_x), y(_y) {
    
}  

Vector2f Vector2f::operator* (const float _value)const {
    return Vector2f(x *_value ,y*_value);
}

Vector2f Vector2f::operator/ (const float _value)const {
    return Vector2f(x /_value ,y/_value);
}

Vector2f Vector2f::operator+ (const Vector2f& _vector)const {
    return Vector2f(x +_vector.x, y + _vector.y);
}

Vector2f Vector2f::operator- (const Vector2f& _vector)const {
    return Vector2f(x -_vector.x, y - _vector.y);
}

void Vector2f::operator-= (const Vector2f& _vector) {
    x -= _vector.x;
    y -= _vector.y;
}

void Vector2f::operator+= (const Vector2f& _vector) {
    x += _vector.x;
    y += _vector.y;
}

void Vector2f::operator*= (const float _value){
    x *= _value;
    y*= _value;
}

void Vector2f::operator/= (const float _value){
    x /= _value;
    y /= _value;
}

bool Vector2f::operator== (const Vector2f& _vector)const{
    return x == _vector.x &&  y == _vector.y;
}

float Vector2f::Magnitude()const {
    return = Sqrt(x*x,y*y);
}
   
float Vector2f::SqrMagnitude()const {
    return = (x*x,y*y);
}

Vector2f Vector2f::GetNormalized()const {
    return this/Magnitude();
}

Vector2f Vector2f::Lerp(const Vector2f& _fromVector,const Vector2f& _toVector, float _alpha){
    return _fromVector + (_toVector - _fromVector) * _alpha;
}

float Vector2f::Dot(const Vector2f& _a, const Vector2f& _b){
    float result = 0;
    result = Sqrt((_a.x * _b.x) + (_a.y * _b.y));
    return result;
}

float Vector2f::Dot(float _magnitudeVectorA, float _magnitudeVectorB, float _angle){
    
    float result = 0;
    result = _magnitudeVectorA*_magnitudeVectorB * Cos(_angle);
    return result;
}

}