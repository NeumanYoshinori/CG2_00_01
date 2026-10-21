#pragma once
#include "Primitive.h"

class Plane : public Primitive {
public:
	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;
};

