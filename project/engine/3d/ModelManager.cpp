#include "ModelManager.h"

using namespace std;

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ModelManager::Initialize(DirectXBase* dxBase) {
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxBase);
}
void ModelManager::LoadModel(const std::string& filePath) {
	// 読み込み済みモデルを検索
	if (models.contains(filePath)) {
		// 読み込み済みなら早期return
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	unique_ptr<Model> model = make_unique<Model>();
	model->Initialize(modelCommon, "resources", filePath);

	// モデルをmapコンテナに格納する
	models.insert(make_pair(filePath, move(model)));
}

Model* ModelManager::FindModel(const string& filePath) {
	// 読み込み済みモデルを検索
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}
