#pragma once
#include "Enemy.h"
#include "Bullet.h"

namespace Collider {
	// 弾と敵の当たり判定を取得
	void CheckBulletEnemyCollision(Bullet* bullet, Enemy* enemy);
};

