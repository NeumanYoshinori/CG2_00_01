#include "Collider.h"

using namespace MathFunction;

void Collider::CheckBulletEnemyCollision(Bullet* bullet, Enemy* enemy) {
	if (MathFunction::IsCollision(bullet->GetAABB(), enemy->GetAABB())) {
		bullet->OnCollision();
		enemy->OnCollision();
	}
}
