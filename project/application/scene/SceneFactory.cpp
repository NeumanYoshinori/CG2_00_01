#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"

using namespace std;

unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName) {
	// 次のシーンを生成
	std::unique_ptr<BaseScene> newScene;

	if (sceneName == "TITLE") {
		newScene = make_unique<TitleScene>();
	}
	else if (sceneName == "GAMEPLAY") {
		newScene = make_unique<GamePlayScene>();
	}

	return newScene;
}
