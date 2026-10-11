#pragma once
#include "ModelCommon.h"

class Cylinder : public ModelCommon {
public:
	// 初期化
	void Initialize(const std::string& filename);

	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	// 分割数
	const uint32_t kDivide_ = 64;
};

