#pragma once
#include "ModelCommon.h"

class Plane : public ModelCommon {
public:
	// 初期化
	void Initialize(const std::string& filename) override;

	// 描画
	void Draw() override;

private:
	void CreateVertexData() override;
};

