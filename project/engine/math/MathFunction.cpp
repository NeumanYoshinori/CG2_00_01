#include "MathFunction.h"
#include <cmath>
#include <cassert>

using namespace std;

namespace MathFunction {
    Vector2& operator+=(Vector2& lhv, const Vector2& rhv) {
        lhv.x += rhv.x;
        lhv.y += rhv.y;
        return lhv;
    }

    const Vector3 operator+(const Vector3& v1, const Vector3& v2) {
        Vector3 temp(v1);
        return temp += v2;
    }

    const Vector3 operator-(const Vector3& v1, const Vector3& v2) {
        Vector3 temp(v1);
        return temp -= v2;
    }

    const Vector3 operator*(const Vector3& v1, const float f) {
        Vector3 temp(v1);
        return temp *= f;
    }

    const Vector3 operator/(const Vector3& v, float f) {
        Vector3 temp(v);
        return temp /= f;
    }

    Vector3& operator+=(Vector3& lhv, const Vector3& rhv) {
        lhv.x += rhv.x;
        lhv.y += rhv.y;
        lhv.z += rhv.z;
        return lhv;
    }

    Vector3& operator-=(Vector3& lhv, const Vector3& rhv) {
        lhv.x -= rhv.x;
        lhv.y -= rhv.y;
        lhv.z -= rhv.z;
        return lhv;
    }

    Vector3& operator*=(Vector3& v, float s) {
        v.x *= s;
        v.y *= s;
        v.z *= s;
        return v;
    }

    Vector3& operator/=(Vector3& v, float s) {
        v.x /= s;
        v.y /= s;
        v.z /= s;
        return v;
    }

    Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm) {
        Matrix4x4 result{};

        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 4; j++) {
                for (size_t k = 0; k < 4; k++) {
                    result.m[i][j] += lhm.m[i][k] * rhm.m[k][j];
                }
            }
        }
        lhm = result;
        return lhm;
    }

    Vector3 operator+(const Vector3& v) { return v; }
    Vector3 operator-(const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); }

    Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
        Matrix4x4 result = m1;

        return result *= m2;
    }

    Quaternion& operator+=(Quaternion& q0, const Quaternion& q1) {
        q0.x += q1.x;
        q0.y += q1.y;
        q0.z += q1.z;
        q0.w += q1.w;

        return q0;
    }

    Quaternion& operator*=(Quaternion& q, float s) {
        q.x *= s;
        q.y *= s;
        q.z *= s;
        q.w *= s;

        return q;
    }

    const Quaternion operator+(const Quaternion& q0, const Quaternion& q1) {
        Quaternion temp(q0);

        return temp += q1;
    }

    const Quaternion operator*(const Quaternion& q, const float f) {
        Quaternion temp(q);

        return temp *= f;
    }

    const Quaternion operator*(const float f, const Quaternion& q) {
        Quaternion temp(q);

        return temp *= f;
    }

    Quaternion operator-(const Quaternion& q) { return Quaternion(-q.x, -q.y, -q.z, -q.w); }

    // 単位行列の作成
    Matrix4x4 MakeIdentity4x4() {
        Matrix4x4 result = {};

        result.m[0][0] = 1.0f; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
        result.m[1][0] = 0.0f; result.m[1][1] = 1.0f; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
        result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1.0f; result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
        Matrix4x4 result = {};
        result.m[0][0] = scale.x; result.m[0][1] = 0.0f;	result.m[0][2] = 0.0f;	  result.m[0][3] = 0.0f;
        result.m[1][0] = 0.0f;	  result.m[1][1] = scale.y; result.m[1][2] = 0.0f;	  result.m[1][3] = 0.0f;
        result.m[2][0] = 0.0f;	  result.m[2][1] = 0.0f;	result.m[2][2] = scale.z; result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f;	  result.m[3][1] = 0.0f;	result.m[3][2] = 0.0f;	  result.m[3][3] = 1.0f;

        return result;
    }

    // X軸回転行列
    Matrix4x4 MakeRotateXMatrix(float radian) {
        Matrix4x4 result = {};
        result.m[0][0] = 1.0f; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
        result.m[1][0] = 0.0f; result.m[1][1] = cos(radian); result.m[1][2] = sin(radian); result.m[1][3] = 0.0f;
        result.m[2][0] = 0.0f; result.m[2][1] = -sin(radian); result.m[2][2] = cos(radian); result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

        return result;
    }

    // Y軸回転行列
    Matrix4x4 MakeRotateYMatrix(float radian) {
        Matrix4x4 result = {};
        result.m[0][0] = cos(radian); result.m[0][1] = 0.0f; result.m[0][2] = -sin(radian); result.m[0][3] = 0.0f;
        result.m[1][0] = 0.0f; result.m[1][1] = 1.0f; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
        result.m[2][0] = sin(radian); result.m[2][1] = 0.0f; result.m[2][2] = cos(radian); result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

        return result;
    }

    // Z軸回転行列
    Matrix4x4 MakeRotateZMatrix(float radian) {
        Matrix4x4 result = {};
        result.m[0][0] = cos(radian); result.m[0][1] = sin(radian); result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
        result.m[1][0] = -sin(radian); result.m[1][1] = cos(radian); result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
        result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1.0f; result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 MakeRotateMatrix(const Vector3& radian) {
        return MakeRotateYMatrix(radian.y) * MakeRotateXMatrix(radian.x) * MakeRotateZMatrix(radian.z);
    }

    // 平行移動行列
    Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
        Matrix4x4 result = {};
        result.m[0][0] = 1.0f;		  result.m[0][1] = 0.0f;		result.m[0][2] = 0.0f;		  result.m[0][3] = 0.0f;
        result.m[1][0] = 0.0f;		  result.m[1][1] = 1.0f;		result.m[1][2] = 0.0f;		  result.m[1][3] = 0.0f;
        result.m[2][0] = 0.0f;		  result.m[2][1] = 0.0f;		result.m[2][2] = 1.0f;		  result.m[2][3] = 0.0f;
        result.m[3][0] = translate.x; result.m[3][1] = translate.y; result.m[3][2] = translate.z; result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
        // 拡大縮小行列
        Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

        // XYZ軸回転行列
        Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);

        // 平行移動行列
        Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

        return scaleMatrix * rotateMatrix * translateMatrix;
    }

    Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion) {
        Matrix4x4 result = {};

        result.m[0][0] = powf(quaternion.w, 2.0f) + powf(quaternion.x, 2.0f) - powf(quaternion.y, 2.0f) - powf(quaternion.z, 2.0f); result.m[0][1] = 2 * (quaternion.x * quaternion.y + quaternion.w * quaternion.z); result.m[0][2] = 2 * (quaternion.x * quaternion.z - quaternion.w * quaternion.y); result.m[0][3] = 0.0f;
        result.m[1][0] = 2 * (quaternion.x * quaternion.y - quaternion.w * quaternion.z); result.m[1][1] = powf(quaternion.w, 2.0f) - powf(quaternion.x, 2.0f) + powf(quaternion.y, 2.0f) - powf(quaternion.z, 2.0f); result.m[1][2] = 2 * (quaternion.y * quaternion.z + quaternion.w * quaternion.x); result.m[1][3] = 0.0f;
        result.m[2][0] = 2 * (quaternion.x * quaternion.z + quaternion.w * quaternion.y); result.m[2][1] = 2 * (quaternion.y * quaternion.z - quaternion.w * quaternion.x); result.m[2][2] = powf(quaternion.w, 2.0f) - powf(quaternion.x, 2.0f) - powf(quaternion.y, 2.0f) + powf(quaternion.z, 2.0f); result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
        Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
        Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
        Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

        return scaleMatrix * rotateMatrix * translateMatrix;
    }

    Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
        Matrix4x4 result = { 1.0f / aspectRatio * (1.0f / tan(fovY / 2.0f)), 0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f / tan(fovY / 2.0f), 0.0f, 0.0f,
                            0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f,
                            0.0f, 0.0f, -nearClip * farClip / (farClip - nearClip), 0.0f };

        return result;
    }

    // 逆行列
    Matrix4x4 Inverse(const Matrix4x4& m) {
        float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2] -
            m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2] -
            m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2] +
            m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2] +
            m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2] -
            m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2] -
            m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0] +
            m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

        Matrix4x4 result = {};
        result.m[0][0] = 1.0f / A * (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);
        result.m[0][1] = 1.0f / A * (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);
        result.m[0][2] = 1.0f / A * (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);
        result.m[0][3] = 1.0f / A * (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);
        result.m[1][0] = 1.0f / A * (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][2]);
        result.m[1][1] = 1.0f / A * (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);
        result.m[1][2] = 1.0f / A * (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);
        result.m[1][3] = 1.0f / A * (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);
        result.m[2][0] = 1.0f / A * (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);
        result.m[2][1] = 1.0f / A * (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);
        result.m[2][2] = 1.0f / A * (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);
        result.m[2][3] = 1.0f / A * (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);
        result.m[3][0] = 1.0f / A * (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);
        result.m[3][1] = 1.0f / A * (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);
        result.m[3][2] = 1.0f / A * (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1]);
        result.m[3][3] = 1.0f / A * (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

        return result;
    }

    Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
        Matrix4x4 result = { 2.0f / (right - left), 0.0f, 0.0f, 0.0f,
                                0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f / (farClip - nearClip), 0.0f,
                                (left + right) / (left - right), (top + bottom) / (bottom - top), nearClip / (nearClip - farClip), 1.0f };

        return result;
    }

    Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
        return { width / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -height / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, maxDepth - minDepth, 0.0f, left + width / 2.0f, top + height / 2.0f, minDepth, 1.0f };
    }

    // 長さ(ノルム)
    float Length(const Vector3& v) {
        return sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2));
    }

    bool IsCollision(const AABB& a, const AABB& b) {
        if(a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z) {
            // 衝突
            return true;
        }

        return false;
    }

    Matrix4x4 Transpose(const Matrix4x4& m) {
        Matrix4x4 result = {};

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m.m[j][i];
            }
        }

        return result;
    }

    // 正規化
    Vector3 Normalize(const Vector3& v) {
        float len = Length(v);
        Vector3 result = {};

        result.x = v.x / len;
        result.y = v.y / len;
        result.z = v.z / len;

        return result;
    }

    // 座標変換
    Vector3 TransformNormal(const Vector3& vector, const Matrix4x4& matrix4x4) {
        Vector3 result = {};
        result.x = vector.x * matrix4x4.m[0][0] + vector.y * matrix4x4.m[1][0] + vector.z * matrix4x4.m[2][0] + 1.0f * matrix4x4.m[3][0];
        result.y = vector.x * matrix4x4.m[0][1] + vector.y * matrix4x4.m[1][1] + vector.z * matrix4x4.m[2][1] + 1.0f * matrix4x4.m[3][1];
        result.z = vector.x * matrix4x4.m[0][2] + vector.y * matrix4x4.m[1][2] + vector.z * matrix4x4.m[2][2] + 1.0f * matrix4x4.m[3][2];
        float w = vector.x * matrix4x4.m[0][3] + vector.y * matrix4x4.m[1][3] + vector.z * matrix4x4.m[2][3] + 1.0f * matrix4x4.m[3][3];
        assert(w != 0.0f);
        result /= w;

        return result;
    }

    // 内積
    float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

    float Dot(const Quaternion& q0, const Quaternion& q1) { return q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w; }

    // 線形補間(Vector3)
    Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
        Vector3 result = {};
        result.x = (1.0f - t) * a.x + t * b.x;
        result.y = (1.0f - t) * a.y + t * b.y;
        result.z = (1.0f - t) * a.z + t * b.z;

        return result;
    }

    // 線形補間(float)
    float Lerp(float x1, float x2, float t) { return (1.0f - t) * x1 + t * x2; }

    // 球面線形補間(Quaternion)
    Quaternion Slerp(Quaternion q0, Quaternion q1, float t) {
        // 内積を求める
        float dot = Dot(q0, q1);

        if (dot < 0) {
            q0 = -q0; // もう片方の回転を利用する
            dot = -dot; // 内積も反転
        }

        // なす角を求める
        float theta = acos(dot);

        // θの角度からsinθを求める
        float sinTheta = sin(theta);
        // サイン(θ(1-t))を求める
        float sinThetaFrom = sin((1.0f - t) * theta);
        // サインθtを求める
        float sinThetaTo = sin(t * theta);

        // thetaとsinを使って補間係数scale0,scale1を求める
        float scale0 = sinThetaFrom / sinTheta;
        float scale1 = sinThetaTo / sinTheta;

        return scale0 * q0 + scale1 * q1;
    }
}
