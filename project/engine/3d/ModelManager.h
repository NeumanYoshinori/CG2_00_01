#pragma once
#include <map>
#include <string>
#include <memory>
#include "ModelCommon.h"

// モデルマネージャー
class ModelManager {
public:
	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Finalize();

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="filePath">モデルファイルのパス</param>
	void LoadModel(const std::string& filePath);

	// プリミティブ作成
	void CreatePrimitive(const std::string& name, const std::string& type, const std::string& filePath);

	/// <summary>
	/// モデルの検索
	/// </summary>
	/// <param name="filePath">モデルファイルのパス</param>
	/// <returns></returns>
	ModelCommon* FindModel(const std::string& filePath);

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class ModelManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit ModelManager(ConstructorKey) {}

private:
	// インスタンス
	static std::unique_ptr<ModelManager> instance_;

	// モデルデータ
	std::map<std::string, std::unique_ptr<ModelCommon>> models_;

	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<ModelManager>;
};