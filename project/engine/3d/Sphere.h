#pragma once
#include "Primitive.h"

// 球
class Sphere : public Primitive {
public: // メンバ関数
	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	const uint32_t kSubdivision_ = 32; // 分割数
};

