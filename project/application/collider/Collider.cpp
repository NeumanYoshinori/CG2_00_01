#include "Collider.h"

using namespace MathFunction;

void Collider::CheckBulletEnemyCollision(Bullet* bullet, Enemy* enemy) {
	if (MathFunction::IsCollision(bullet->GetAABB(), enemy->GetAABB())) {
		bullet->OnCollision();
		enemy->OnCollision();
	}
}

/*void Collider::CreateAABB(AABB aabb, Matrix4x4 viewProjectionMatrix, Matrix4x4 viewportMatrix) {
	Vector3 vertex[8] = {};
	vertex[0] = { aabb.min.x, aabb.min.y, aabb.min.z };
	vertex[1] = { aabb.max.x, aabb.min.y, aabb.min.z };
	vertex[2] = { aabb.min.x, aabb.max.y, aabb.min.z };
	vertex[3] = { aabb.max.x, aabb.max.y, aabb.min.z };
	vertex[4] = { aabb.min.x, aabb.min.y, aabb.max.z };
	vertex[5] = { aabb.max.x, aabb.min.y, aabb.max.z };
	vertex[6] = { aabb.min.x, aabb.max.y, aabb.max.z };
	vertex[7] = { aabb.max.x, aabb.max.y, aabb.max.z };

	for (int i = 0; i < 8; ++i) {
		vertex[i] = TransformNormal(TransformNormal(vertex[i], viewProjectionMatrix), viewportMatrix);
	}

	CollisionLine collisionLine;
	collisionLine.lines_[0].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y));
	collisionLine.lines_[1].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[2].x, vertex[2].y));
	collisionLine.lines_[2].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[4].x, vertex[4].y));
	collisionLine.lines_[3].SetStartAndEnd(Vector2(vertex[1].x, vertex[1].y), Vector2(vertex[3].x, vertex[3].y));
	collisionLine.lines_[4].SetStartAndEnd(Vector2(vertex[1].x, vertex[1].y), Vector2(vertex[5].x, vertex[5].y));
	collisionLine.lines_[5].SetStartAndEnd(Vector2(vertex[2].x, vertex[2].y), Vector2(vertex[3].x, vertex[3].y));
	collisionLine.lines_[6].SetStartAndEnd(Vector2(vertex[2].x, vertex[2].y), Vector2(vertex[6].x, vertex[6].y));
	collisionLine.lines_[7].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y));
	collisionLine.lines_[8].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y));
	collisionLine.lines_[9].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y));
	collisionLine.lines_[10].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y));
	collisionLine.lines_[11].SetStartAndEnd(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y));
	aabbLines_.push_back(collisionLine);
}

void Collider::Draw() {
}*/
