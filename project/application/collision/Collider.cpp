#include "Collider.h"

using namespace MathFunction;

namespace Collider {
	void CheckBulletEnemyCollision(Bullet* bullet, Enemy* enemy) {
		if (MathFunction::IsCollision(bullet->GetAABB(), enemy->GetAABB())) {
			bullet->OnCollision();
			enemy->OnCollision();
		}
	}
}
