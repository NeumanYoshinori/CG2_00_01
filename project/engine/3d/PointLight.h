#pragma once
#include "MathFunction.h"
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "SrvManager.h"

class PointLight {
public:
	// ポイントライト
	struct ConstBufferData {
		Vector4 color;
		Vector3 position;
		float intensity;
		float radius;
		float decay;
		float padding[2];
	};

	void SetColor(Vector4 color) { color_ = color; }
	Vector4 GetColor() const { return color_; }

	void SetPosition(Vector3 position) { position_ = position; }
	Vector3 GetPosition() const { return position_; }

	void SetIntensity(float intensity) { intensity_ = intensity; }
	float GetIntensity() const { return intensity_; }

	void SetRadius(float radius) { radius_ = radius; }
	float GetRadius() const { return radius_; }

	void SetDecay(float decay) { decay_ = decay; }
	float GetDecay() const { return decay_; }

private:
	Vector4 color_;

	Vector3 position_;

	float intensity_;

	float radius_;

	float decay_;
};
