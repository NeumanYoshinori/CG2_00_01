#include "TitleScene.h"
#include "SceneManager.h"

using namespace std;

void TitleScene::Initialize() {
	// インスタンス取得
	input_ = Input::GetInstance();

	textureManager_ = TextureManager::GetInstance();

	imGuiManager_ = ImGuiManager::GetInstance();
}

void TitleScene::Finalize() {
}

void TitleScene::Update() {
	// ENTERキーを押したら
	if (input_->TriggerKey(DIK_R)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}

	imGuiManager_->Begin();
	imGuiManager_->End();
}

void TitleScene::Draw() {
	// SRVマネージャ描画前処理
	SrvManager::GetInstance()->PreDraw();
}

void TitleScene::ImGuiDraw() {
	// ImGui受付開始
	ImGuiManager::GetInstance()->Draw();
}
