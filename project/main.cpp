#include "Game.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Game game;

	// ゲームの初期化
	game.Initialize();

	while (true) { // ゲームループ
		// 毎フレーム更新
		game.Update();
		// 終了リクエストが来たら抜ける
		if (game.IsEndRequest()) {
			break;
		}
		// 描画
		game.Draw();
	}
	// ゲームの終了
	game.Finalize();

	return 0;
}