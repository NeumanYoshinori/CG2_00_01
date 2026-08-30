#include "LevelDataLoader.h"
#include <fstream>

const std::string LevelDataLoader::kDefaultBaseDirectory = "resources/";
const std::string LevelDataLoader::kExtension = ".json";

LevelData* LevelDataLoader::LoadJson(const std::string filename) {
	const std::string fullpath = kDefaultBaseDirectory + filename + kExtension;

	// ファイルストリーム
	std::ifstream file;

	// ファイルを開く
	file.open(fullpath);
	// ファイルオープン失敗をチェック
	if (file.fail()) {
		assert(0);
	}

	// JSON文字列から解凍したデータ
	nlohmann::json jsonData;

	// 解凍
	file >> jsonData;

	// 正しいレベルデータファイルかチェック
	assert(jsonData.is_object());
	assert(jsonData.contains("name"));
	assert(jsonData["name"].is_string());

	// "name"を文字列として取得
	std::string name =
		jsonData["name"].get<std::string>();
	assert(name.compare("scene") == 0);

	// レベルデータ格納用インスタンスを生成
	LevelData* levelData = new LevelData();
	// オブジェクト内の全オブジェクトを走査
	for (nlohmann::json& object : jsonData["objects"]) {
		assert(object.contains("type"));
		ScanAllObject(object, levelData);
	}

	return levelData;
}

void LevelDataLoader::ScanAllObject(nlohmann::json& object, LevelData* levelData) {
	// 種別を取得
	std::string type = object["type"].get<std::string>();

	// MESH
	if (type.compare("MESH") == 0) {
		// 要素追加
		levelData->objects.emplace_back(LevelData::ObjectData{});
		// 今追加した要素の参照を得る
		LevelData::ObjectData& objectData = levelData->objects.back();

		if (object.contains("file_name")) {
			// ファイル名
			objectData.fileName = object["file_name"];
		}

		// トランスフォームのパラメータ読み込み
		nlohmann::json& transform = object["transform"];
		// 平行移動
		objectData.translation.x = (float)transform["translation"][0];
		objectData.translation.y = (float)transform["translation"][2];
		objectData.translation.z = (float)transform["translation"][1];
		// 回転角
		objectData.rotation.x = -(float)transform["rotation"][0];
		objectData.rotation.y = -(float)transform["rotation"][2];
		objectData.rotation.z = -(float)transform["rotation"][1];
		// スケーリング
		objectData.scaling.x = (float)transform["scaling"][0];
		objectData.scaling.y = (float)transform["scaling"][2];
		objectData.scaling.z = (float)transform["scaling"][1];
	}

	if (object.contains("children")) {
		for (nlohmann::json& childObject : object["children"]) {
			ScanAllObject(childObject, levelData);
		}
	}
}
