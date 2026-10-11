#pragma once
#include "ModelCommon.h"

class Ring : public ModelCommon {
public:
	// 初期化
	void Initialize(const std::string& filename);

	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	const uint32_t kDivide_ = 64;
};

