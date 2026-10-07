#pragma once
#include "Enemy.h"
#include "Bullet.h"

class Collider {
public:
	// 銃と敵の当たり判定を取得.
	static void CheckBulletEnemyCollision(Bullet* bullet, Enemy* enemy);

/*	// 作成
	void CreateAABB(AABB aabb, Matrix4x4 viewProjectionMatrix, Matrix4x4 viewportMatrix);

	// 描画*/
	void DrawAABB();

protected:
};

