#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

struct AABB {
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};

namespace MathFunction {
	Vector2& operator+=(Vector2& lhv, const Vector2& rhv);

	const Vector3 operator+(const Vector3& v1, const Vector3& v2);
	const Vector3 operator-(const Vector3& v1, const Vector3& v2);
	const Vector3 operator*(const Vector3& v1, const float f);
	const Vector3 operator/(const Vector3& v, float f);

	Vector3& operator+=(Vector3& lhv, const Vector3& rhv);
	Vector3& operator-=(Vector3& lhv, const Vector3& rhv);
	Vector3& operator*=(Vector3& v, float s);
	Vector3& operator/=(Vector3& v, float s);

	Vector3 operator+(const Vector3& v);
	Vector3 operator-(const Vector3& v);

	// 代入演算子オーバーロード
	Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);

	// 2項演算子オーバーロード
	Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

	// 単位行列の作成
	Matrix4x4 MakeIdentity4x4();

	// 拡大縮小行列
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	// X軸回転行列
	Matrix4x4 MakeRotateXMatrix(float radian);
	// Y軸回転行列
	Matrix4x4 MakeRotateYMatrix(float radian);
	// Z軸回転行列
	Matrix4x4 MakeRotateZMatrix(float radian);

	// XYZ軸回転行列
	Matrix4x4 MakeRotateMatrix(const Vector3& radian);

	// 平行移動行列
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	// アフィン変換
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	// 透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farclip);

	// 逆行列
	Matrix4x4 Inverse(const Matrix4x4& m);

	// 正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	// ビューポート行列を作成
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	// 長さ
	float Length(const Vector3& v);

	bool IsCollision(const AABB& a, const AABB& b);

	Matrix4x4 Transpose(const Matrix4x4& m);

	// 正規化
	Vector3 Normalize(const Vector3& v);

	// 座標返還
	Vector3 TransformNormal(const Vector3& vector, const Matrix4x4& matrix4x4);
}

