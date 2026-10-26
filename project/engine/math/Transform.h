#pragma once
#include "Vector3.h"
#include "Quaternion.h"

// 座標変換
struct EulerTransform {
	Vector3 scale;
	Vector3 rotate; // Eulerでの回転
	Vector3 translate;
};

struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};

