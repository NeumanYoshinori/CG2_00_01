#pragma once
#include "Primitive.h"

class Cylinder : public Primitive {
public:
	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	// 分割数
	const uint32_t kDivide_ = 64;
};

