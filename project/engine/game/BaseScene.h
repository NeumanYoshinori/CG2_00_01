#pragma once

// シーン基底クラス
class BaseScene {
public: // メンバ関数
	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finalize();

	// 更新
	virtual void Update();

	// 描画
	virtual void Draw();

	// 仮想デストラクタ
	virtual ~BaseScene() = default;
};

