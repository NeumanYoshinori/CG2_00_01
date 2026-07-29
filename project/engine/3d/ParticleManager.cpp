#include "ParticleManager.h"
#include <wrl.h>
#include "Logger.h"
#include "MathFunction.h"
#include <numbers>
#include "Sphere.h"
#include "Plane.h"
#include "Ring.h"
#include "Cylinder.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace Logger;
using namespace MathFunction;
using namespace numbers;

unique_ptr<ParticleManager> ParticleManager::instance_ = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<ParticleManager>(ConstructorKey());
	}
	return instance_.get();
}

void ParticleManager::Finalize() {
	instance_.reset();
}

void ParticleManager::Initialize() {
	// シングルトンインスタンスを取得
	dxBase_ = DirectXBase::GetInstance();
	srvManager_ = SrvManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// 乱数生成器の初期化
	randomEngine_ = mt19937(seedGenerator_());

	// グラフィックスパイプライン生成
	GenerateGraphicsPipeline();
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

	// Δtを設定
	const float kDeltaTime = 1.0f / 60.0f;

	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup.numInstance = 0;

		for (auto particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end(); ) {
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator); // 生存期間を過ぎていたら更新せず描画対象にしない
				continue;
			}

			// 速度を適用
			(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;

			// 経過時間を加算
			(*particleIterator).currentTime += kDeltaTime;

			Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
			Matrix4x4 rotateXMatrix = MakeRotateXMatrix((*particleIterator).transform.rotate.x);
			Matrix4x4 rotateYMatrix = MakeRotateYMatrix((*particleIterator).transform.rotate.y);
			Matrix4x4 rotateZMatrix = MakeRotateZMatrix((*particleIterator).transform.rotate.z);
			Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
			Matrix4x4 worldMatrix = scaleMatrix * rotateXMatrix * rotateYMatrix * rotateZMatrix * billboardMatrix * translateMatrix;

			Matrix4x4 worldViewProjectionMatrix = worldMatrix * viewMatrix * projectionMatrix;

			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

			if (particleGroup.numInstance < kNumMaxInstance_) {
				particleGroup.instancingData[particleGroup.numInstance].WVP = worldViewProjectionMatrix;
				particleGroup.instancingData[particleGroup.numInstance].World = worldMatrix;
				particleGroup.instancingData[particleGroup.numInstance].color = (*particleIterator).color;
				particleGroup.instancingData[particleGroup.numInstance].color.w = alpha;
				particleGroup.instancingData[particleGroup.numInstance].flipX = particleGroup.flipX;
				particleGroup.instancingData[particleGroup.numInstance].flipY = particleGroup.flipY;
				++particleGroup.numInstance;
			}
			++particleIterator;
		}

		// インスタンス数を渡す
		particleGroup.primitive->SetNumInstance(particleGroup.numInstance);
	}
}

void ParticleManager::Draw() {
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(graphicsPipelineState_.Get());
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto& [name, particleGroup] : particleGroups_) {
		if (particleGroup.numInstance == 0) {
			continue;
		}
		commandList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(particleGroup.srvIndex));

		// プリミティブの描画
		particleGroup.primitive->Draw();
	}
}

void ParticleManager::CreateParticleGroup(const string type, const string name, const string textureFilePath, int32_t flipX, int32_t flipY) {
	// 登録済みの名前かチェックしてreturn
	if (particleGroups_.contains(name)) {
		return;
	}

	ParticleGroup& particleGroup = particleGroups_[name];
	// テクスチャを読み込む
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	// インスタンシング用のリソースの生成
	particleGroup.instancingResource = dxBase_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance_);
	particleGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData));
	// インスタンシング用にSRVを確保してSRVインデックスを記録
	particleGroup.srvIndex = srvManager_->Allocate();
	particleGroup.flipX = flipX;
	particleGroup.flipY = flipY;
	// 頂点データ作成
	if (type == "Sphere") {
		particleGroup.primitive = make_unique<Sphere>();
	}
	if (type == "Plane") {
		particleGroup.primitive = make_unique<Plane>();
	}
	else if (type == "Ring") {
		particleGroup.primitive = make_unique<Ring>();
	}
	else if (type == "Cylinder") {
		particleGroup.primitive = make_unique<Cylinder>();
	}
	// プリミティブの初期化
	particleGroup.primitive->Initialize(textureFilePath, particleGroup.numInstance);
	// SRV生成（StructuredBuffer用設定）
	srvManager_->CreateSRVforStructuredBuffer(particleGroup.srvIndex, particleGroup.instancingResource.Get(), kNumMaxInstance_, sizeof(ParticleForGPU));
}

// パーティクル生成関数
ParticleManager::Particle ParticleManager::MakeNewParticle(mt19937& randomEngine, const Vector3& scale, const Vector3& rotate, const Vector3& translate, const Vector3& velocity, const Vector4& color, float lifeTime) {
	uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	ParticleManager::Particle particle{};
	particle.transform.scale = scale; // 横に潰す
	particle.transform.rotate = rotate;
	Vector3 randomTranslate{ distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.transform.translate = translate + randomTranslate;
	particle.velocity = velocity; // 動かない
	particle.color = color;
	particle.lifeTime = lifeTime;
	particle.currentTime = 0;
	return particle;
}

void ParticleManager::Emit(const std::string name, const Vector3& size, const Vector3& angle, const Vector3& position, const Vector3& velocity, const Vector4& color, float lifeTime, uint32_t count) {
	assert(particleGroups_.contains(name));
	ParticleGroup& particleGroup = particleGroups_[name];

	for (uint32_t i = 0; i < count; ++i) {
		particleGroup.particles.push_back(MakeNewParticle(randomEngine_, size, angle, position, velocity, color, lifeTime));
	}
}

void ParticleManager::CreateRootSignature() {
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
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

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	// RootSignature(パーティクル用)
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
		IID_PPV_ARGS(&rootSignature_));
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

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get(); // RootSignature
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
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}
