#pragma once
#include <string>
#include "ModelCommon.h"

// 球
class Sphere : public ModelCommon {
public: // メンバ関数
	// 初期化
	void Initialize(const std::string& filename) override;

	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	const uint32_t kSubdivision_ = 32; // 分割数
};

