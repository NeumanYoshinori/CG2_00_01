#include "ModelManager.h"
#include "Model.h"
#include "Sphere.h"
#include "Plane.h"
#include "Ring.h"
#include "Cylinder.h"

using namespace std;

unique_ptr<ModelManager> ModelManager::instance_ = nullptr;

ModelManager* ModelManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<ModelManager>(ConstructorKey());
	}
	return instance_.get();
}

void ModelManager::Finalize() {
	instance_.reset();
}

void ModelManager::LoadModel(const std::string& filePath) {
	// 読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		// 読み込み済みなら早期return
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	unique_ptr<Model> model = make_unique<Model>();
	model->Initialize("resources", filePath);

	// モデルをmapコンテナに格納する
	models_.insert(make_pair(filePath, move(model)));
}

void ModelManager::CreatePrimitive(const std::string& name, const std::string& type, const std::string& filePath) {
	// 読み込み済みモデルを検索
	if (primitives_.contains(name)) {
		// 読み込み済みなら早期return
		return;
	}
	
	// モデルの生成
	unique_ptr<Primitive> primitive;
	if (type == "Sphere") {
		primitive = make_unique<Sphere>();
	}
	else if (type == "Plane") {
		primitive = make_unique<Plane>();
	}
	else if (type == "Ring") {
		primitive = make_unique<Ring>();
	}
	else if (type == "Cylinder") {
		primitive = make_unique<Cylinder>();
	}

	primitive->Initialize(filePath);

	primitives_.insert(make_pair(name, move(primitive)));
}

Model* ModelManager::FindModel(const string& filePath) {
	// 読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		return models_.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}

Primitive* ModelManager::FindPrimitive(const string& name) {
	if (primitives_.contains(name)) {
		return primitives_.at(name).get();
	}

	// 名前一致なし
	return nullptr;
}
