#pragma once
#include "Line.h"

class HitBox {
public:
	// 初期化
	void Initialize(Camera* camera);

	// 更新
	void Update(const AABB& aabb);

	// 描画
	void Draw();

private:
	// 線の数
	static const uint32_t kNumLines = 12;
	// 線
	std::array<std::unique_ptr<Line>, kNumLines> lines_;

	// 頂点
	Vector3 vertices_[8];

	// 頂点を結ぶ組み合わせ
	const int edges_[kNumLines][2] = {
		{ 0, 1 }, { 0, 2 }, { 1, 3 }, { 2, 3 },
		{ 4, 5 }, { 4, 6 }, { 5, 7 }, { 6, 7 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	};
};

