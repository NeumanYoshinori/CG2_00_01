#include "ParticleManager.h"
#include <wrl.h>
#include "Logger.h"
#include "TextureManager.h"
#include "MathFunction.h"
#include <numbers>

using namespace std;
using namespace Microsoft::WRL;
using namespace Logger;
using namespace MathFunction;
using namespace numbers;

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ParticleManager::Initialize(DirectXBase* dxBase, SrvManager* srvManager, Camera* camera) {
	// 引数で受け取ってメンバ変数に記録する
	dxBase_ = dxBase;
	srvManager_ = srvManager;
	camera_ = camera;

	// 乱数生成器の初期化
	randomEngine_ = mt19937(seedGenerator_());

	// グラフィックスパイプライン生成
	GenerateGraphicsPipeline();

	// 
	CreateVertexData();

	CreateMaterialData();
}

void ParticleManager::Update() {
	// ビルボード行列
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(pi_v<float>);
	Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera_->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	// ビュー行列
	Matrix4x4 viewMatrix = camera_->GetViewMatrix();
	// プロジェクション行列
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();

	// 場
	AccelerationField accelerationField;
	accelerationField.acceleration = { 15.0f, 0.0f, 0.0f };
	accelerationField.area.min = { -1.0f, -1.0f, -1.0f };
	accelerationField.area.max = { 1.0f, 1.0f, 1.0f };

	// Δtを設定
	const float kDeltaTime = 1.0f / 60.0f;

	for (auto& [name, particleGroup] : particleGroups) {
		particleGroup.numInstance = 0;

		for (auto particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end(); ) {
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator); // 生存期間を過ぎていたら更新せず描画対象にしない
				continue;
			}

			// Fieldの範囲内位のParticleには加速度を適用する
			if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
				(*particleIterator).velocity += accelerationField.acceleration * kDeltaTime;
			}
			// 速度を適用
			(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;

			// 経過時間を加算
			(*particleIterator).currentTime += kDeltaTime;

			Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
			Matrix4x4 worldMatrix = scaleMatrix * billboardMatrix * translateMatrix;

			Matrix4x4 worldViewProjectionMatrix = worldMatrix * viewMatrix * projectionMatrix;

			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

			if (particleGroup.numInstance < kNumMaxInstance) {
				particleGroup.instancingData[particleGroup.numInstance].WVP = worldViewProjectionMatrix;
				particleGroup.instancingData[particleGroup.numInstance].World = worldMatrix;
				particleGroup.instancingData[particleGroup.numInstance].color = (*particleIterator).color;
				particleGroup.instancingData[particleGroup.numInstance].color.w = alpha;
				++particleGroup.numInstance;
			}
			++particleIterator;
		}
	}
}

void ParticleManager::Draw() {
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	for (auto& [name, particleGroup] : particleGroups) {
		if (particleGroup.numInstance == 0) {
			continue;
		}
		srvManager_->SetGraphicsRootDescriptorTable(2, particleGroup.materialData.textureIndex);
		srvManager_->SetGraphicsRootDescriptorTable(1, particleGroup.srvIndex);
		commandList->DrawInstanced(6, particleGroup.numInstance, 0, 0);
	}
}

void ParticleManager::CreateParticleGroup(const string name, const string textureFilePath) {
	// 登録済みの名前かチェックしてassert
	assert(!particleGroups.contains(name));

	ParticleGroup& particleGroup = particleGroups[name];
	// マテリアルデータにテクスチャファイルパスを設定
	particleGroup.materialData.textureFilePath = textureFilePath;
	// テクスチャを読み込む
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	// マテリアルデータにテクスチャのSRVインデックスを記録
	particleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	// インスタン寝具用のリソースの生成
	particleGroup.instancingResource = dxBase_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	particleGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData));
	// インスタン寝具用にSRVを確保してSRVインデックスを記録
	particleGroup.srvIndex = srvManager_->Allocate();
	// SRV生成（StructuredBuffer用設定）
	srvManager_->CreateSRVforStructuredBuffer(particleGroup.srvIndex, particleGroup.instancingResource.Get(), kNumMaxInstance, sizeof(ParticleForGPU));
}

// パーティクル生成関数
ParticleManager::Particle MakeNewParticle(mt19937& randomEngine, const Vector3& translate) {
	uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	uniform_real_distribution<float> distColor(0.0f, 1.0f);
	uniform_real_distribution<float> distTime(1.0f, 3.0f);
	ParticleManager::Particle particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	Vector3 randomTranslate{ distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.transform.translate = translate + randomTranslate;
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
	assert(particleGroups.contains(name));
	ParticleGroup& particleGroup = particleGroups[name];

	for (uint32_t i = 0; i < count; ++i) {
		particleGroup.particles.push_back(MakeNewParticle(randomEngine_, position));
	}
}

void ParticleManager::CreateRootSignature() {
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	// RootSingature(パーティクル用)
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = dxBase_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

void ParticleManager::GenerateGraphicsPipeline() {
	CreateRootSignature();

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "COLOR";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	ComPtr<IDxcBlob> vertexShaderBlob = dxBase_->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlob = dxBase_->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Depthの書き込みを行わない
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	HRESULT hr = dxBase_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}

void ParticleManager::CreateVertexData() {
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 頂点リソースを作る
	vertexResource = dxBase_->CreateBufferResource(sizeof(VertexData) * 6);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	
	vertexData[0].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 0.0f };
	vertexData[0].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[1].position = { -1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 1.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 0.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[3].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 0.0f, 1.0f };
	vertexData[3].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[4].position = { -1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 1.0f, 0.0f };
	vertexData[4].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[5].position = { -1.0f, -1.0f, 0.0f, 1.0f };
	vertexData[5].texcoord = { 1.0f, 1.0f };
	vertexData[5].normal = { 0.0f, 0.0f, 1.0f };
}

void ParticleManager::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// マテリアルデータの初期値を書き込む
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
}
