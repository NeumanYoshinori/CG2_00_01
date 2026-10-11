#pragma once
#include "Enemy.h"
#include "Bullet.h"

class Collider {
public:
	// 銃と敵の当たり判定を取得.
	static void CheckBulletEnemyCollision(Bullet* bullet, Enemy* enemy);
};

