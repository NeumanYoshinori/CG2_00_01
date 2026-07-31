#pragma once
#include "Primitive.h"
#include "DirectXBase.h"

class Plane : public Primitive {
public:
	// 描画
	void Draw() override;

private:
	void CreateVertexData() override;
};

