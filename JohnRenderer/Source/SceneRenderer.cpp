#include "pch.h"
#include "SceneRenderer.h"
#include "Application.h"
#include "JohnShader.h"
#include "SelectionOutlineShader.h"
#include "Sky.h"
#include "Texture.h"
#include "PointLight.h"
#include "JohnCamera.h"
#include "DepthOnlyShader.h"
SceneRenderer::~SceneRenderer()
{
	m_PrePostProcessTarget->ReleaseDevice();
	m_ViewportRenderTarget->ReleaseDevice();
	m_ToneMap.reset();
}

void SceneRenderer::Initialize()
{
	auto device = Application::Get().GetDevice ();
	auto context = Application::Get().GetContext ();
	m_PBRShader = std::make_shared<JohnShader>();
	m_PBRShader->LoadFromFiles (L"PBRVS.cso", L"PBRPS.cso");

	m_SelectionOutlineShader = std::make_shared <SelectionOutlineShader>();
	m_SelectionOutlineShader->LoadFromFiles (L"SimpleVS.cso", L"SelectionOutlinePS.cso");

	m_DepthOnlyShader = std::make_shared<DepthOnlyShader>();
	m_DepthOnlyShader->LoadFromFiles (L"SimpleVS.cso");


	m_Sky = std::make_shared<Sky>();
	m_Sky->LoadFromFile ("Content/kloofendal_48d_partly_cloudy_puresky_1k.hdr");
	m_SkyMesh = GeometricPrimitive::CreateGeoSphere (context, 2.f, 3);

	m_SkyEffect = std::make_unique<DX::SkyboxEffect>(device);

	m_SkyMesh->CreateInputLayout(m_SkyEffect.get(), m_SkyInputLayout.ReleaseAndGetAddressOf ());

	m_SkyEffect->SetTexture(m_Sky->GetEnvMap().Get());

	m_CommonStates = std::make_unique<CommonStates>(device);

	m_PrePostProcessTarget = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_ViewportRenderTarget = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_B8G8R8A8_UNORM);


	m_PrePostProcessTarget->SetDevice(device);
	m_ViewportRenderTarget->SetDevice(device);
	int width, height;
	Application::Get().GetBackBufferSize (width, height);

	RECT rc = { 0,0,width, height };
	m_PrePostProcessTarget->SetWindow(rc);
	m_ViewportRenderTarget->SetWindow(rc);

	CD3D11_TEXTURE2D_DESC depthDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		width,
		height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	ThrowIfFailed(
		device->CreateTexture2D(
			&depthDesc,
			nullptr,
			m_DepthStencil.ReleaseAndGetAddressOf()
		)
	);



	m_StandardState = m_CommonStates->DepthDefault ();



	ThrowIfFailed (
		device->CreateDepthStencilView (
			m_DepthStencil.Get(),
			nullptr,
			m_DepthStencilView.ReleaseAndGetAddressOf ()
		)
	);

	m_ToneMap = std::make_unique<DirectX::ToneMapPostProcess>(device);

	m_ToneMap->SetOperator(ToneMapPostProcess::None);
	m_ToneMap->SetTransferFunction(ToneMapPostProcess::SRGB);
	m_ToneMap->SetHDRSourceTexture(m_PrePostProcessTarget->GetShaderResourceView());

	m_PBREffect = std::make_unique<PBREffect>(device);
	m_PBREffect->EnableDefaultLighting();
	m_Sphere = GeometricPrimitive::CreateGeoSphere (context, 1, 3, false);
	m_Sphere->CreateInputLayout(m_PBREffect.get(),
		m_InputLayout.ReleaseAndGetAddressOf ());

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	m_Sky->GetSpecularIBL ()->GetDesc (&desc);
	m_PBREffect->SetIBLTextures(m_Sky->GetDiffuseIBL ().Get (), desc.TextureCube.MipLevels, m_Sky->GetSpecularIBL ().Get());

	CreateDefaultAssets ();

	D3D11_BUFFER_DESC lightDesc = {};
	lightDesc.ByteWidth = sizeof(GPUPointLight) * MaxPointLights;
	lightDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	lightDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightDesc.StructureByteStride = sizeof(GPUPointLight);

	ThrowIfFailed (
		device->CreateBuffer (&lightDesc, nullptr, m_PointLightBuffer.ReleaseAndGetAddressOf ())
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = MaxPointLights;

	ThrowIfFailed (
		device->CreateShaderResourceView (m_PointLightBuffer.Get(), &srvDesc, m_PointLightSRV.ReleaseAndGetAddressOf ())
	);


	D3D11_DEPTH_STENCIL_DESC dDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());

	dDesc.DepthEnable = FALSE;
	dDesc.StencilEnable = TRUE;
	dDesc.StencilWriteMask = 0xFF;
	dDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	

	ThrowIfFailed (
		device->CreateDepthStencilState (&dDesc, m_StencilWriteState.ReleaseAndGetAddressOf ())
	);

	dDesc.StencilEnable = TRUE;
	dDesc.StencilReadMask = 0xFF;
	dDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	ThrowIfFailed (
		device->CreateDepthStencilState (&dDesc, m_StencilMaskState.ReleaseAndGetAddressOf ())
	);


}

void SceneRenderer::DrawScene(Scene* InScene, Matrix view, Matrix proj)
{
	auto context = Application::Get().GetContext ();



	m_PBRShader->SetView (view);
	m_PBRShader->SetProj (proj);
	m_CachedView = view;
	m_CachedProj = proj;

	m_PBRShader->SetEnvironmentTextures (m_Sky->GetDiffuseIBL (), m_Sky->GetSpecularIBL ());


	Matrix viewProj = view * proj;

	int width, height;
	D3D11_TEXTURE2D_DESC texDesc = {};
	m_ViewportRenderTarget->GetRenderTarget()->GetDesc(&texDesc);

	D3D11_VIEWPORT viewport
	{
		0.f,
		0.f,
		(float)texDesc.Width,
		(float)texDesc.Height,
		0.1f,
		1.f
	};
	auto rt = m_PrePostProcessTarget->GetRenderTargetView();
	context->OMSetRenderTargets (1, &rt, m_DepthStencilView.Get());
	context->RSSetViewports(1, &viewport);



	context->RSSetState (GetCurrentRSState ());
	context->OMSetDepthStencilState(m_StandardState.Get (), 0xFF);

	SetupLights (InScene);


	for(auto& actor : InScene->m_Actors)
	{
		actor->Draw(m_PBRShader);
	}

	m_SkyEffect->SetView(view);
	m_SkyEffect->SetProjection(proj);
	m_SkyMesh->Draw(m_SkyEffect.get(), m_SkyInputLayout.Get());

	if (Application::Get().IsActorSelected ())
	{
		Application::Get().BeginDebugEvent (L"Draw Selection Outline");
		DrawSelectionOutline (Application::Get().GetSelectedActor ());
		Application::Get().EndDebugEvent ();
	}
}

void SceneRenderer::SetupLights(Scene* InScene)
{
	auto context = Application::Get().GetContext ();
	std::vector<GPUPointLight> pointLights;
	int index = 0;
	for(auto& light : InScene->m_PointLights)
	{
		if(index >= MaxPointLights)
		{
			break;
		}

		GPUPointLight gpuLight = {};
		gpuLight.Position = light->GetPosition ();
		gpuLight.Color = light->GetColor ();
		gpuLight.Intensity = light->GetIntensity ();
		pointLights.push_back (gpuLight);
		index++;
	}

	int numGPUPointLights = pointLights.size();

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ThrowIfFailed (
		context->Map(m_PointLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes)
	);

	size_t sizeinBytes = numGPUPointLights * sizeof(GPUPointLight);
	memcpy_s(mappedRes.pData, sizeinBytes, pointLights.data(), sizeinBytes);
	context->Unmap(m_PointLightBuffer.Get(), 0);

	context->PSSetShaderResources (6, 1, m_PointLightSRV.GetAddressOf ());
	m_PBRShader->SetNumPointLights(numGPUPointLights);
}

void SceneRenderer::DrawPostProcess()
{
	auto context = Application::Get().GetContext ();
	auto rt = m_ViewportRenderTarget->GetRenderTargetView();
	context->OMSetRenderTargets(1, &rt, nullptr);

	m_ToneMap->Process(context);
}

void SceneRenderer::DrawSelectionOutline(std::shared_ptr<Actor> selectedActor)
{
	auto context = Application::Get().GetContext ();

	ID3D11RenderTargetView* nullRTV = {nullptr};
	context->OMSetDepthStencilState (m_StencilWriteState.Get(),1 );
	context->OMSetRenderTargets (0, nullptr, m_DepthStencilView.Get());

	auto camera = Application::Get().GetCamera ();
	m_SelectionOutlineShader->SetView (camera->GetViewMatrix ());
	m_SelectionOutlineShader->SetProj(camera->GetProjectionMatrix ());

	m_DepthOnlyShader->SetView (camera->GetViewMatrix ());
	m_DepthOnlyShader->SetProj (camera->GetProjectionMatrix ());
	selectedActor->DrawDepth(m_DepthOnlyShader);

	

	context->OMSetDepthStencilState (m_StencilMaskState.Get(), 1);
	auto rt = m_PrePostProcessTarget->GetRenderTargetView();
	context->OMSetRenderTargets (1, &rt, m_DepthStencilView.Get());

	selectedActor->DrawOutline (m_SelectionOutlineShader);

}

void SceneRenderer::SwitchToWireframe()
{
	m_CurrentDrawMode = EDrawMode::Wireframe;;
}

void SceneRenderer::SwitchToLit()
{
	m_CurrentDrawMode = EDrawMode::Lit;
}

void SceneRenderer::OnViewportResize(int width, int height)
{
	auto device = Application::Get().GetDevice();
	RECT size =
	{
		0,
		0,
		width,
		height
	};
	m_PrePostProcessTarget->SetWindow(size);
	m_ViewportRenderTarget->SetWindow(size);

	CD3D11_TEXTURE2D_DESC depthDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		width,
		height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	ThrowIfFailed(
		device->CreateTexture2D(
			&depthDesc,
			nullptr,
			m_DepthStencil.ReleaseAndGetAddressOf()
		)
	);

	CD3D11_DEPTH_STENCIL_VIEW_DESC dsv(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT);

	ThrowIfFailed(
		device->CreateDepthStencilView(
			m_DepthStencil.Get(),
			&dsv,
			m_DepthStencilView.ReleaseAndGetAddressOf()
		)
	);

	m_ToneMap->SetHDRSourceTexture(m_PrePostProcessTarget->GetShaderResourceView());
}


DX::RenderTexture* SceneRenderer::GetPrePostProccessTarget() const
{
	return m_PrePostProcessTarget.get();
}

DX::RenderTexture * SceneRenderer::GetViewportRenderTarget() const
{
	return m_ViewportRenderTarget.get();
}

void SceneRenderer::CreateDefaultAssets()
{
	m_DefaultDiffuse = CreateDefaultTexture (0x7f, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	m_DefaultNormal = CreateDefaultTexture (0x7f7f, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_DefaultRoughness = CreateDefaultTexture (0x7f, DXGI_FORMAT_R8_UNORM);
	m_DefaultMetallic = CreateDefaultTexture (0x00, DXGI_FORMAT_R8_UNORM);
}

std::shared_ptr<Texture> SceneRenderer::CreateDefaultTexture(uint16_t color, DXGI_FORMAT format)
{
	auto device = Application::Get().GetDevice ();

	D3D11_SUBRESOURCE_DATA data = { &color, sizeof(uint16_t), 0 };

	ComPtr<ID3D11Texture2D> texture2D;
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = texDesc.Height = texDesc.MipLevels = texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ThrowIfFailed (
		device->CreateTexture2D 
		(
			&texDesc,
			&data,
			texture2D.ReleaseAndGetAddressOf ()
		)
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	ComPtr<ID3D11ShaderResourceView> srv;
	
	ThrowIfFailed (
		device->CreateShaderResourceView (texture2D.Get(), &srvDesc, srv.ReleaseAndGetAddressOf ())
	);

	return std::make_shared<Texture>(texture2D, srv);
}

void SceneRenderer::ClearRenderTargets()
{
	float clearColor[4] = { .2f, .2f, .2f, 1.f };

	auto context = Application::Get().GetContext();

	auto PrePostProcess = m_PrePostProcessTarget->GetRenderTargetView();
	context->ClearRenderTargetView(PrePostProcess, clearColor);
	context->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

void SceneRenderer::RecompileShaders()
{
	m_PBRShader->Recompile();
}

ID3D11RasterizerState* SceneRenderer::GetCurrentRSState() const
{
	switch(m_CurrentDrawMode)
	{
	case EDrawMode::Lit:
		return m_CommonStates->CullCounterClockwise();
		break;
	case EDrawMode::Wireframe:
		return m_CommonStates->Wireframe();
		break;
	}
}
