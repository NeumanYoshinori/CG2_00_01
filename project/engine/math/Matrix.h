#pragma once

struct Matrix4x4 {
	float m[4][4];
};

struct Vector3 {
	float x;
	float y;
	float z;
};

const Vector3 operator+(const Vector3& v1, const Vector3& v2);

const Vector3 operator*(const Vector3& v1, const float f);

Vector3& operator+=(Vector3& lhv, const Vector3& rhv);
Vector3& operator-=(Vector3& lhv, const Vector3& rhv);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);

// 代入演算子オーバーロード
Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);

// 2項演算子オーバーロード
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v);

class Matrix {
public:
	// 単位行列の作成
	Matrix4x4 MakeIdentity4x4();

	// 行列の積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	// 拡大縮小行列
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	// X軸回転行列
	Matrix4x4 MakeRotateXMatrix(float radian);
	// Y軸回転行列
	Matrix4x4 MakeRotateYMatrix(float radian);
	// Z軸回転行列
	Matrix4x4 MakeRotateZMatrix(float radian);

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
};
