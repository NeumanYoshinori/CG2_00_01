#include "PostEffect.h"
#include "SrvManager.h"
#include "RenderTextureCommon.h"
#include "Logger.h"

using namespace Microsoft::WRL;
using namespace Logger;
using namespace std;

unique_ptr<PostEffect> PostEffect::instance_ = nullptr;

PostEffect* PostEffect::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<PostEffect>(ConstructorKey());
	}
	return instance_.get();
}

void PostEffect::Finalize() {
	instance_.reset();
}

void PostEffect::Initialize(DirectXBase* dxBase) {
	dxBase_ = dxBase;

	renderTextureResource_ = RenderTextureCommon::GetInstance()->GetRenderTextureResource();
	srvIndex_ = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforTexture2D(srvIndex_, renderTextureResource_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, false);

	GenerateGraphicsPipeline(L"resources/shaders/FullScreen.PS.hlsl", PostEffectType::FullScreen);
	GenerateGraphicsPipeline(L"resources/shaders/Grayscale.PS.hlsl", PostEffectType::Grayscale);
	GenerateGraphicsPipeline(L"resources/shaders/Vignette.PS.hlsl", PostEffectType::Vignette);

	CreateMaterialData();
}

void PostEffect::Draw() {
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(graphicsPipelineState_[static_cast<int>(postEffectType_)].Get());
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_));
	commandList->DrawInstanced(3, 1, 0, 0);
}

void PostEffect::CreateRootSignature() {
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	// RootSignature
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

void PostEffect::GenerateGraphicsPipeline(wstring psName, PostEffectType postEffectType) {
	CreateRootSignature();

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	ComPtr<IDxcBlob> vertexShaderBlob = dxBase_->CompileShader(L"resources/shaders/FullScreen.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlob = dxBase_->CompileShader(psName,
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get(); // RootSignature
	// InputLayoutの設定
	graphicsPipelineStateDesc.InputLayout.pInputElementDescs = nullptr;
	graphicsPipelineStateDesc.InputLayout.NumElements = 0;
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
	graphicsPipelineStateDesc.DepthStencilState.DepthEnable = false;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	HRESULT hr = dxBase_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState_[static_cast<int>(postEffectType)]));
	assert(SUCCEEDED(hr));
}

void PostEffect::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// マテリアルデータの初期値を書き込む
	materialData_->color = { 1.0f, 1.0f, 1.0f };
	materialData_->useSepia = false;
}