#pragma once
#include "MathFunction.h"

class SpotLight {
public:
	struct ConstBufferData {
		Vector4 color;
		Vector3 position;
		float intensity;
		Vector3 direction;
		float distance;
		float decay;
		float cosAngle;
		float padding[2];
	};
};

