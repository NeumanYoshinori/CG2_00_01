#pragma once
#include <string>
#include <assimp/scene.h>
#include "ModelCommon.h"

// 3Dモデル
class Model : public ModelCommon {
public:
	// 初期化
	void Initialize(const std::string& filename) override;

	// 描画
	void Draw() override;

private:
	// 頂点データ作成
	void CreateVertexData() override;

	// .objファイルの読み取り
	static ModelData LoadModelFile(const std::string& filename);

	// ノードを読む
	static Node ReadNode(aiNode* node);
};

