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
	unique_ptr<ModelCommon> model = make_unique<Model>();
	model->Initialize(filePath);

	// モデルをmapコンテナに格納する
	models_.insert(make_pair(filePath, move(model)));
}

void ModelManager::CreatePrimitive(const std::string& name, const std::string& type, const std::string& filePath) {
	// 読み込み済みモデルを検索
	if (models_.contains(name)) {
		// 読み込み済みなら早期return
		return;
	}
	
	// モデルの生成
	unique_ptr<ModelCommon> model;
	if (type == "Sphere") {
		model = make_unique<Sphere>();
	}
	else if (type == "Plane") {
		model = make_unique<Plane>();
	}
	else if (type == "Ring") {
		model = make_unique<Ring>();
	}
	else if (type == "Cylinder") {
		model = make_unique<Cylinder>();
	}

	model->Initialize(filePath);

	models_.insert(make_pair(name, move(model)));
}

ModelCommon* ModelManager::FindModel(const string& filePath) {
	// 読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		return models_.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}
