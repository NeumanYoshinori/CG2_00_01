#pragma once
#include "Line.h"

class HitBox {
public:
	// 初期化
	void Initialize(float width, float height, float depth, Camera* camera);

	// 更新
	void Update();

	// 描画
	void Draw();

	void SetTranslate(Vector3 translate);

private:
	static const uint32_t kNumLines = 12;
	std::array<std::unique_ptr<Line>, kNumLines> lines_;
};

