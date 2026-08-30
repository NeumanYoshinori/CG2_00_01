#pragma once
#include <string>
#include "Vector3.h"
#include <externals/json/json.hpp>
#include <vector>

struct LevelData {
	struct ObjectData {
		nlohmann::json fileName;
		Vector3 scaling;
		Vector3 rotation;
		Vector3 translation;
	};

	std::vector<ObjectData> objects;
};

class LevelDataLoader {
public:
	static const std::string kDefaultBaseDirectory;

	static const std::string kExtension;

	static LevelData* LoadJson(const std::string filename);

	static void ScanAllObject(nlohmann::json& object, LevelData* levelData);
};
