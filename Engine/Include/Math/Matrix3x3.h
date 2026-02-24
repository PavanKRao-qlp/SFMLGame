#pragma once
#include "Math/MathUtils.h"
#include "Math/Vector.h"

namespace Umbra::Math {

    // Row-major 3x3 affine matrix for 2D transforms.
    //
    // Layout for a TRS(translation, angleDeg, scale) matrix:
    //
    //   | m[0][0]  m[0][1]  m[0][2] |   | sx*cosA  -sy*sinA   tx |
    //   | m[1][0]  m[1][1]  m[1][2] | = | sx*sinA   sy*cosA   ty |
    //   | m[2][0]  m[2][1]  m[2][2] |   |    0         0       1  |
    //
    // Column 0 encodes the scaled X-axis; column 1 the scaled Y-axis.
    // Column 2 (tx, ty) is the translation.
    struct Matrix3x3f {
        float m[3][3];

        // Default: zero matrix. Use Identity() for an identity.
        inline Matrix3x3f() {
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    m[i][j] = 0.f;
        }

        static inline Matrix3x3f Identity() {
            Matrix3x3f mat;
            mat.m[0][0] = 1.f;
            mat.m[1][1] = 1.f;
            mat.m[2][2] = 1.f;
            return mat;
        }

        static inline Matrix3x3f Translate(Vector2f _t) {
            Matrix3x3f mat = Identity();
            mat.m[0][2]    = _t.x;
            mat.m[1][2]    = _t.y;
            return mat;
        }

        static inline Matrix3x3f Rotate(float _angleDeg) {
            Matrix3x3f mat = Identity();
            float rad      = DegreeToRadian(_angleDeg);
            float c        = Cos(rad);
            float s        = Sin(rad);
            mat.m[0][0]    = c;
            mat.m[0][1]    = -s;
            mat.m[1][0]    = s;
            mat.m[1][1]    = c;
            return mat;
        }

        static inline Matrix3x3f Scale(Vector2f _s) {
            Matrix3x3f mat = Identity();
            mat.m[0][0]    = _s.x;
            mat.m[1][1]    = _s.y;
            return mat;
        }

        // Builds T * R * S in one shot (scale first, then rotate, then translate).
        static inline Matrix3x3f TRS(Vector2f _t, float _angleDeg, Vector2f _s) {
            float rad   = DegreeToRadian(_angleDeg);
            float c     = Cos(rad);
            float s     = Sin(rad);
            Matrix3x3f mat;
            mat.m[0][0] = c * _s.x;
            mat.m[0][1] = -s * _s.y;
            mat.m[0][2] = _t.x;
            mat.m[1][0] = s * _s.x;
            mat.m[1][1] = c * _s.y;
            mat.m[1][2] = _t.y;
            mat.m[2][0] = 0.f;
            mat.m[2][1] = 0.f;
            mat.m[2][2] = 1.f;
            return mat;
        }

        // Matrix multiplication: this * _rhs
        inline Matrix3x3f operator*(const Matrix3x3f& _rhs) const {
            Matrix3x3f r;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    r.m[i][j] = m[i][0] * _rhs.m[0][j] + m[i][1] * _rhs.m[1][j] + m[i][2] * _rhs.m[2][j];
            return r;
        }

        inline Matrix3x3f& operator*=(const Matrix3x3f& _rhs) {
            *this = *this * _rhs;
            return *this;
        }

        // Transform a point (applies translation).
        inline Vector2f TransformPoint(Vector2f _p) const {
            return Vector2f(m[0][0] * _p.x + m[0][1] * _p.y + m[0][2],
                            m[1][0] * _p.x + m[1][1] * _p.y + m[1][2]);
        }

        // Transform a direction vector (ignores translation).
        inline Vector2f TransformVector(Vector2f _v) const {
            return Vector2f(m[0][0] * _v.x + m[0][1] * _v.y, m[1][0] * _v.x + m[1][1] * _v.y);
        }

        // Analytic inverse for a 2D affine matrix (bottom row must be [0,0,1]).
        // Returns identity if the matrix is degenerate (zero determinant).
        inline Matrix3x3f GetInverse() const {
            float a      = m[0][0], b = m[0][1], tx = m[0][2];
            float c      = m[1][0], d = m[1][1], ty = m[1][2];
            float det    = a * d - b * c;
            if (Abs(det) < EPSILON)
                return Identity();
            float inv    = 1.f / det;
            Matrix3x3f r;
            r.m[0][0]    =  d * inv;
            r.m[0][1]    = -b * inv;
            r.m[0][2]    = (b * ty - d * tx) * inv;
            r.m[1][0]    = -c * inv;
            r.m[1][1]    =  a * inv;
            r.m[1][2]    = (c * tx - a * ty) * inv;
            r.m[2][0]    = 0.f;
            r.m[2][1]    = 0.f;
            r.m[2][2]    = 1.f;
            return r;
        }

        // Decompose: translation stored in column 2.
        inline Vector2f GetTranslation() const {
            return Vector2f(m[0][2], m[1][2]);
        }

        // Decompose: rotation in degrees.
        // Column 0 = (sx*cosA, sx*sinA) so atan2(m[1][0], m[0][0]) gives the angle.
        inline float GetRotationDeg() const {
            return RadianToDegree(Atan2(m[1][0], m[0][0]));
        }

        // Decompose: non-uniform scale.
        // |column 0| = scaleX,  |column 1| = scaleY.
        inline Vector2f GetScale() const {
            float sx = Sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);
            float sy = Sqrt(m[0][1] * m[0][1] + m[1][1] * m[1][1]);
            return Vector2f(sx, sy);
        }

        inline bool operator==(const Matrix3x3f& _rhs) const {
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    if (Abs(m[i][j] - _rhs.m[i][j]) > EPSILON)
                        return false;
            return true;
        }

        inline bool operator!=(const Matrix3x3f& _rhs) const {
            return !(*this == _rhs);
        }
    };

} // namespace Umbra::Math
