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
#include "Material.h"
#include "Geometry/Brush.h"

static constexpr float g_MaxWorldCoord = (64 * 1024);
static constexpr float g_MinWorldCoord = -(64 * 1024);

#define SIDE_FRONT      0
#define SIDE_ON         2
#define SIDE_BACK       1
#define SIDE_CROSS      -2

float GetVectorAtIndex(Vector3 v, int index)
{
	if(index == 0)
	{
		return v.x;
	}
	else if (index == 1)
	{
		return v.y;
	}
	else if (index == 2)
	{
		return v.z;
	}
	else
	{
		return v.x;
	}
}
SceneRenderer::~SceneRenderer()
{
	m_PrePostProcessTarget->ReleaseDevice();
	m_ViewportRenderTarget->ReleaseDevice();
	m_ToneMap.reset();
}

void SceneRenderer::CreateBrush()
{
	Vector3 mins, maxs;
	mins.x = -2;
	mins.y = -2;
	mins.z = -2;

	maxs.x = 2;
	maxs.y = 2;
	maxs.z = 2;

	Vector3 points[4][2];

	if(maxs.x < mins.x && maxs.y < mins.y && maxs.z < mins.z)
	{
		return;
	}
	m_Brush = new Brush();

	points[0][0].x = mins.x;
	points[0][0].y = maxs.y;

	points[1][0].x = mins.x;
	points[1][0].y = maxs.y;

	points[2][0].x = maxs.x;
	points[2][0].y = maxs.y;

	points[3][0].x = maxs.x;
	points[3][0].y = mins.y;

	for(int i = 0; i < 4; i++)
	{
		points[i][0].z = mins.z;
		points[i][1].x = points[i][0].x;
		points[i][1].y = points[i][0].y;
		points[i][1].z = maxs.z;

	}


	for(int i = 0; i < 4; i++)
	{
		BrushFace* f = new BrushFace();
		f->next = m_Brush->BrushFaces;
		m_Brush->BrushFaces = f;
		int j = (i + 1) % 4;

		f->PlanePoints[0] = points[j][1];
		f->PlanePoints[1] = points[i][1];
		f->PlanePoints[2] = points[i][0];
	}

	BrushFace* f = new BrushFace();
	f->next = m_Brush->BrushFaces;
	m_Brush->BrushFaces = f;

	f->PlanePoints[0] = points[0][1];
	f->PlanePoints[1] = points[1][1];
	f->PlanePoints[2] = points[2][1];

	BrushFace* f2 = new BrushFace();
	f->next = m_Brush->BrushFaces;
	m_Brush->BrushFaces = f;
	f2->PlanePoints[0] = points[2][0];
	f2->PlanePoints[1] = points[1][0];
	f2->PlanePoints[2] = points[0][0];


	BuildBrush  ();



}




void SceneRenderer::BuildBrush()
{
	BuildBrushWindings  ();
}

void SceneRenderer::BuildBrushWindings()
{
	Winding* w;
	BrushFace* f;
	Vector3 v;

	m_Brush->Mins.x = m_Brush->Mins.y = m_Brush->Mins.z = 9999999;
	m_Brush->Maxs.x = m_Brush->Maxs.y = m_Brush->Maxs.z = -9999999;

	MakeBrushFacePlanes  ();

	f = m_Brush->BrushFaces;

	for( ; f; f = f->next)
	{
		delete f->face_Winding;
		w = f->face_Winding = MakeFaceWinding  (f);

	}

}

void SceneRenderer::MakeBrushFacePlanes()
{
	BrushFace* f;
	for(f = m_Brush->BrushFaces; f; f = f->next)
	{
		MakeFacePlane  (f);
	}
}

void SceneRenderer::MakeFacePlane(BrushFace* f)
{
	Vector3 t1, t2, t3;

	
	t1.x = f->PlanePoints[0].x - f->PlanePoints[1].x;
	t2.x = f->PlanePoints[2].x - f->PlanePoints[1].x;
	t3.x = f->PlanePoints[1].x;

	t1.y = f->PlanePoints[0].y - f->PlanePoints[1].y;
	t2.y = f->PlanePoints[2].y - f->PlanePoints[1].y;
	t3.y = f->PlanePoints[1].y;

	t1.z = f->PlanePoints[0].z - f->PlanePoints[1].z;
	t2.z = f->PlanePoints[2].z - f->PlanePoints[1].z;
	t3.z = f->PlanePoints[1].z;

	Vector3 cross = t1.Cross  (t2);
	f->plane.x = cross.x;
	f->plane.y = cross.y;
	f->plane.z = cross.z;

	f->plane.Normalize();

	float dot = t3.Dot  (f->plane.Normal  ());
	f->plane.w = dot;
	
}

Winding* SceneRenderer::MakeFaceWinding(BrushFace* face)
{
	Winding* w;
	BrushFace* clip;
	Plane plane;
	bool past;

	w = MakeBaseForPlane(&face->plane);
	
	past = false;
	for(clip = m_Brush->BrushFaces; clip && w; clip = clip->next)
	{
		if(clip == face)
		{
			past = true;
			continue;
		}
		if(face->plane.Normal  ().Dot(clip->plane.Normal  ()) > .999 &&
			fabs(face->plane.w - clip->plane.w) < .01)
		{
			if(past)
			{
				delete w;
				return nullptr;
			}
			continue;
		}
		Vector3 norm = Vector3::Zero - clip->plane.Normal  ();
		plane.x = norm.x;
		plane.y = norm.y;
		plane.z = norm.z;

		plane.w = -clip->plane.w;


	}
}
#define WINDING_SIZE( pt ) ( sizeof( int )*2 + sizeof( float )*5*( pt ) )

Winding* SceneRenderer::MakeBaseForPlane(Plane* p)
{
	
	float max = -g_MaxWorldCoord;
	int x = -1;
	Vector3 norm = p->Normal  ();
	float v = fabs(norm.x);
	if( v > max)
	{
		x = 0;
		max = v;
	}
	v = fabs(norm.y);
	if(v > max)
	{
		x = 1;
		max = v;
	}
	v = fabs(norm.z);
	if(v > max)
	{
		x = 2;
		max = v;
	}
	Vector3 vup = Vector3::Zero;

	switch(x)
	{
	case 0:
	case 1:
		vup.z = 1;
		break;
	case 2:
		vup.x = 1;
		break;
	}

	v = vup.Dot  (p->Normal  ());
	vup = vup + -v * p->Normal  ();
	vup.Normalize  ();

	Vector3 org = p->Normal  () * p->w;

	Vector3 vright = vup.Cross  (p->Normal  ());

	vup *= g_MaxWorldCoord;
	vright *= g_MaxWorldCoord;

	Winding* w = new Winding();
	int size = (sizeof(int) * 2 + sizeof(float) * 5 * (4));
	w->MaxPoints = 4;
	w->NumPoints = 4;

	w->Points[0] = org - vright;
	w->Points[0] += vup;

	w->Points[1] = org + vright;
	w->Points[1] += vup;

	w->Points[2] = org + vright;
	w->Points[2] -= vup;

	w->Points[3] = org - vright;
	w->Points[3] -= vup;

	return w;

}

Winding* SceneRenderer::WindingClip(Winding* in, Plane* split, bool keepon)
{

	int counts[3] = { 0,0,0 };
	float dists[64];
	int sides[64];
	int i;
	Vector3 p1, p2;
	for(i = 0; i < in->NumPoints; i++)
	{
		float dot = in->Points[i].Dot(split->Normal  ());
		dot -= split->w;
		

		dists[i] = dot;

		if(dot > .01)
		{
			sides[i] = 0;
		}
		else if (dot < -.01)
		{
			sides[i] = 1;
		}
		else
		{
			sides[i] = 2;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if(keepon && !counts[0] && !counts[1])
	{
		return in;
	}

	if(!counts[0])
	{
		delete in;
		return nullptr;
	}
	if(!counts[1])
	{
		return in;
	}
	int maxpnts = in->NumPoints + 4;

	Winding* neww = new Winding();
	neww->MaxPoints = maxpnts;

	for(i = 0; i < in->NumPoints; i++)
	{
		p1 = in->Points[i];

		if(sides[i] == SIDE_ON)
		{
			neww->Points[neww->NumPoints] = p1;
			neww->NumPoints++;
			continue;
		}
		if(sides[i] = SIDE_FRONT)
		{
			neww->Points[neww->NumPoints] = p1;
			neww->NumPoints++;
		}
		if(sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
		{
			continue;
		}

		p2 = in->Points[(i + 1) % in->NumPoints];

		float dot = dists[i] / (dists[i] - dists[i + 1]);

		if(split->Normal().x == 1)
		{
			//mid.d
		}

	}
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

	InitializeDefaults  ();

	CreateBrush  ();
}

void SceneRenderer::InitializeDefaults()
{
	auto device = Application::Get().GetDevice  ();
	static const uint32_t s_baseColorPixel = 0x0;

	D3D11_SUBRESOURCE_DATA initData = { &s_baseColorPixel, sizeof(uint32_t), 0 };
	

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> defDiffuse;
	ThrowIfFailed  (device->CreateTexture2D  (&desc, &initData, defDiffuse.ReleaseAndGetAddressOf  ()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ComPtr<ID3D11ShaderResourceView> srvDiff;
	ThrowIfFailed  (device->CreateShaderResourceView  (defDiffuse.Get(), &srvDesc, srvDiff.ReleaseAndGetAddressOf  ()));

	m_DefaultDiffuse = std::make_shared<Texture>(defDiffuse, srvDiff);


	//normal
	static const uint16_t s_norm = 0x7f7f;
	desc.Format = DXGI_FORMAT_R8G8_UNORM;
	initData.pSysMem = &s_norm;
	initData.SysMemPitch = sizeof(uint16_t);

	ComPtr<ID3D11Texture2D> norm;
	ThrowIfFailed  (device->CreateTexture2D  (&desc, &initData, norm.ReleaseAndGetAddressOf  ()));

	srvDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	
	ComPtr<ID3D11ShaderResourceView> normSRV;
	ThrowIfFailed  (device->CreateShaderResourceView  (norm.Get(), &srvDesc, normSRV.ReleaseAndGetAddressOf  ()));

	m_DefaultNormal = std::make_shared<Texture>(norm, normSRV);

	//roughness
	static const uint8_t s_rough = 0x50;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	initData.pSysMem = &s_rough;
	initData.SysMemPitch = sizeof(uint8_t);

	ComPtr<ID3D11Texture2D> rough;
	ThrowIfFailed  (device->CreateTexture2D  (&desc, &initData, rough.ReleaseAndGetAddressOf  ()));

	srvDesc.Format = DXGI_FORMAT_R8_UNORM;

	ComPtr<ID3D11ShaderResourceView> roughSRV;
	ThrowIfFailed  (device->CreateShaderResourceView  (rough.Get(), &srvDesc, roughSRV.ReleaseAndGetAddressOf  ()));

	m_DefaultRoughness = std::make_shared<Texture>(rough, roughSRV);

	//metallic
	static const uint8_t s_metal = 0x0;
	initData.pSysMem = &s_metal;

	ComPtr<ID3D11Texture2D> metal;
	ThrowIfFailed  (device->CreateTexture2D  (&desc, &initData, metal.ReleaseAndGetAddressOf  ()));

	srvDesc.Format = DXGI_FORMAT_R8_UNORM;

	ComPtr<ID3D11ShaderResourceView> metalSRV;
	ThrowIfFailed  (device->CreateShaderResourceView  (rough.Get(), &srvDesc, metalSRV.ReleaseAndGetAddressOf  ()));

	m_DefaultMaterial = std::make_shared<Material>();
	m_DefaultMaterial->SetBaseColorMap(m_DefaultDiffuse);
	m_DefaultMaterial->SetNormalMap  (m_DefaultNormal);
	m_DefaultMaterial->SetRoughnessMap  (m_DefaultRoughness);
	m_DefaultMaterial->SetMetallicMap  (m_DefaultMetallic);

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
