#include "ModelManager.h"

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

Model* ModelManager::FindModel(const string& filePath) {
	// 読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		return models_.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}
