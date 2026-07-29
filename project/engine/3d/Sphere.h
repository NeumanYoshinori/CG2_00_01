#pragma once
#include <string>
#include <wrl.h>
#include "Primitive.h"

class Object3dCommon;

// 球
class Sphere : public Primitive {
public: // メンバ関数
	// 描画
	void Draw() override;

	float GetEnvironmentCoefficient() { return materialData_->environmentCoefficient; }
	void SetEnvironmentCoefficient(float environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	const uint32_t kSubdivision_ = 32; // 分割数
};

