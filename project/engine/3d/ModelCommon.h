#pragma once
#include "DirectXBase.h"

// 3Dモデル共通部
class ModelCommon {
public:
	// 初期化
	void Initialize(DirectXBase* directXBase);

	// getter
	DirectXBase* GetDxBase() const { return dxBase_; }

private:
	DirectXBase* dxBase_;
};

