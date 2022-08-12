//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "JohnMesh.h"
#include "Utilities.h"
#include "renderdoc_app.h"


extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

RENDERDOC_API_1_5_0* rdoc_api = NULL;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}


// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, (void**)&rdoc_api);
		assert(ret == 1);
	}

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

	m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	m_Keyboard = std::make_unique<DirectX::Keyboard>();
	m_Mouse = std::make_unique<DirectX::Mouse>();
	m_Mouse->SetWindow(window);

	m_CameraPos = Vector3(0.f, 0.f, 3.f);
	m_Pitch = 0.f;
	m_Yaw = 0.f;

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

void Game::InitUI(HWND window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());

			
}
#pragma region Init InitializeSky
void Game::InitializeSky()
{
	if(rdoc_api)
	{
		rdoc_api->StartFrameCapture(NULL, NULL);
	}

	



	m_deviceResources->PIXBeginEvent(L"Init Sky");
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto device = m_deviceResources->GetD3DDevice();
	ID3D11UnorderedAccessView* const nullUAV[] = { nullptr };
	m_ComputeSampler = John::CreateSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
	//load and convert equirect to cubemap
	{
		//temp unfiltered cubemap
		John::Texture m_EnvMapUnfiltered = John::CreateTextureCube(device, 1024,1024, DXGI_FORMAT_R16G16B16A16_FLOAT);
		John::CreateTextureUAV(m_EnvMapUnfiltered, 0, device);


		{

			John::ComputeProgram equirectToCubeProgram = John::CreateComputeProgram(John::CompileShader(L"Shaders/Compute/CubemapConversion.hlsl", "main", "cs_5_0"), device);
			John::Texture envTextureEquirect = John::CreateTexture(device, context, John::Image::FromFile("Content/fouriesburg_mountain_midday_8k.hdr"), DXGI_FORMAT_R32G32B32A32_FLOAT,1);


			context->CSSetShaderResources(0, 1, envTextureEquirect.SRV.GetAddressOf());
			context->CSSetUnorderedAccessViews(0, 1, m_EnvMapUnfiltered.UAV.GetAddressOf(), nullptr);
			context->CSSetSamplers(0, 1, m_ComputeSampler.GetAddressOf());
			context->CSSetShader(equirectToCubeProgram.ComputeShader.Get(), nullptr, 0);
			context->Dispatch(envTextureEquirect.Width / 32, envTextureEquirect.Height / 32, 6);

			context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

		}
		context->GenerateMips(m_EnvMapUnfiltered.SRV.Get());
		m_EnvMap = John::CreateTextureCube(device, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);

		for(int arraySlice=0; arraySlice<6; ++arraySlice)
		{
			const UINT subresourceIndex = D3D11CalcSubresource(0, arraySlice, m_EnvMap.Levels);
			context->CopySubresourceRegion(m_EnvMap.texture.Get(), subresourceIndex, 0, 0, 0, m_EnvMapUnfiltered.texture.Get(), subresourceIndex, nullptr);

		}
		struct SpecMapFilterSettingsCB
		{
			float roughness;
			float padding[3];
		};

		John::ComputeProgram spmapProgram = John::CreateComputeProgram(John::CompileShader(L"Shaders/Compute/SPMap.hlsl", "main", "cs_5_0"), device);
		Microsoft::WRL::ComPtr<ID3D11Buffer> spmapCB = John::CreateConstantBuffer<SpecMapFilterSettingsCB>(device);
		
		context->CSSetShaderResources(0, 1, m_EnvMapUnfiltered.SRV.GetAddressOf());
		context->CSSetSamplers(0, 1, m_ComputeSampler.GetAddressOf());
		context->CSSetShader(spmapProgram.ComputeShader.Get(), nullptr, 0);

		const float deltaRoughness = 1.f / std::max(float(m_EnvMap.Levels - 1), 1.f);
		for(UINT level = 1, size = 512; level < m_EnvMap.Levels; ++level, size /=2)
		{
			wchar_t buf[100];
			swprintf_s(buf, 100, L"Level %i", level);
			m_deviceResources->PIXBeginEvent(buf);
			const UINT numGroups = std::max<UINT>(1, size / 32);
			John::CreateTextureUAV(m_EnvMap, level, device);
			
			const SpecMapFilterSettingsCB spMapConstants = { level * deltaRoughness };
			context->UpdateSubresource(spmapCB.Get(), 0, nullptr, &spMapConstants, 0, 0);
			context->CSSetUnorderedAccessViews(0, 1, m_EnvMap.UAV.GetAddressOf(), nullptr);
			context->Dispatch(numGroups, numGroups, 6);
			m_deviceResources->PIXEndEvent();
		}
		context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
	}
	//compute diffuse irradiance map
	{
		John::ComputeProgram irMapProgram = John::CreateComputeProgram(John::CompileShader(L"Shaders/Compute/IrradianceMapGenerator.hlsl", "main", "cs_5_0"), device);
		m_IrradianceMap = John::CreateTextureCube(device, 32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);
		John::CreateTextureUAV(m_IrradianceMap, 0, device);

		context->CSSetShaderResources(0, 1, m_EnvMap.SRV.GetAddressOf());
		context->CSSetSamplers(0, 1, m_ComputeSampler.GetAddressOf());
		context->CSSetUnorderedAccessViews(0, 1, m_IrradianceMap.UAV.GetAddressOf(), nullptr);
		context->CSSetShader(irMapProgram.ComputeShader.Get(), nullptr, 0);
		context->Dispatch(m_IrradianceMap.Width / 32, m_IrradianceMap.Height / 32, 6);
		context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	}

	//compute spec BRDF
	{
		John::ComputeProgram BRDFProgram = John::CreateComputeProgram(John::CompileShader(L"Shaders/Compute/SPBRDF.hlsl", "main", "cs_5_0"), device);

		m_BRDF_LUT = John::CreateTexture(device, 256, 256, DXGI_FORMAT_R16G16_FLOAT, 1);
		
		m_BRDF_Sampler = John::CreateSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);
		John::CreateTextureUAV(m_BRDF_LUT, 0, device);

		context->CSSetUnorderedAccessViews(0, 1, m_BRDF_LUT.UAV.GetAddressOf(), nullptr);
		context->CSSetShader(BRDFProgram.ComputeShader.Get(), nullptr, 0);
		context->Dispatch(m_BRDF_LUT.Width / 32, m_BRDF_LUT.Height / 32, 1);
		context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
	}

	m_Skydome = John::LoadMeshFromFile("Content/skyDome.obj");
	m_Skydome->Build(device);
	m_deviceResources->PIXEndEvent();
	if(rdoc_api)
	{
		rdoc_api->EndFrameCapture(NULL, NULL);
	}
}
#pragma endregion
#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

void Game::TickUI()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Editor");
	bool open = true;
	if(ImGui::BeginTabBar("TabBar"))
	{
		static bool bCameraOpen = true;
		if(ImGui::BeginTabItem("Camera", &open))
		{
			ImGui::DragFloat("Movement Speed", &m_CamSettings.MovementSpeed, .1f, 0.f);
			ImGui::DragFloat("Mouse Sensitivity", &m_CamSettings.MouseLookSensitivity, .1f, 0.f);
			ImGui::DragFloat("Maya Controls Sensitivity", &m_CamSettings.MouseOrbitSensitivity, .1f, 0.f);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();

}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
	if (m_Yaw > 180.f)
	{
		m_Yaw -= 360.f;
	}
	else if (m_Yaw < -180.f)
	{
		m_Yaw += 360.f;
	}

	float y = sinf(m_Pitch);


	XMVECTOR lookAt = m_CameraPos + GetCameraForwardVector();
	Quaternion orientation = GetCameraOrientation();
	Matrix TransMat = Matrix::CreateTranslation(CalculatePosition());
	Matrix RotMat = Matrix::CreateFromQuaternion(orientation);

	m_ViewMatrix = XMMatrixLookAtLH(m_CameraPos, m_LookPosition , GetCameraUpVector());
	m_ViewMatrix.Invert();

	auto rect = m_deviceResources->GetOutputSize();
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	float AspectRatio = (float)width / (float)height;
	m_ProjMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(70.f), AspectRatio, .01f, 100.f);

	Movement(elapsedTime);
    // TODO: Add your game logic here.
    elapsedTime;

	TickUI();	
}

void Game::Movement(float DeltaTime)
{
	auto keyboard = m_Keyboard->GetState();
	auto mouse = m_Mouse->GetState();
	m_Keys.Update(keyboard);
	m_MouseButtons.Update(mouse);
	if (m_Keys.pressed.R)
	{
		const std::vector<D3D11_INPUT_ELEMENT_DESC> meshInputLayout = {
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		m_PBRProgram = John::CreateShaderProgram(
			John::CompileShader(m_PBRProgram.VertFileName, "main", "vs_5_0"),
			John::CompileShader(m_PBRProgram.PixelFileName, "main", "ps_5_0"),
			m_PBRProgram.VertFileName,
			m_PBRProgram.PixelFileName,
			&meshInputLayout,
			m_deviceResources->GetD3DDevice()

		);


	}

	if (CanMoveCamera())
	{
		if(keyboard.LeftAlt)
		{
			Vector2 delta = Vector2(float(mouse.x), float(mouse.y)) * DeltaTime * m_CamSettings.MouseOrbitSensitivity;
			if(mouse.middleButton)
			{
				CameraPan(delta);
			}
			else if(mouse.leftButton)
			{
				CameraOrbit(delta);
			}
			else if(mouse.rightButton)
			{
				CameraZoom(delta);
			}

		}
		else
		{


			Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f) * DeltaTime * m_CamSettings.MouseLookSensitivity;
			m_Pitch -= delta.y;
			m_Yaw += delta.x;

			Vector3 move = Vector3::Zero;

			float velocity = m_CamSettings.MovementSpeed * DeltaTime;

			if (keyboard.E)
			{
				move += (Vector3::Up * velocity);
			}
			if (keyboard.Q)
			{
				move -= Vector3::Up * velocity;
			}
			if (keyboard.A)
			{
				move += GetCameraRightVector() * velocity;
			}
			if (keyboard.D)
			{
				move -= GetCameraRightVector() * velocity;
			}
			if (keyboard.W)
			{
				move += GetCameraForwardVector() * velocity;
			}
			if (keyboard.S)
			{
				move -= GetCameraForwardVector() * velocity;
			}

			if (mouse.scrollWheelValue != 0)
			{
				int x = 5;
			}
			m_CameraPos += move;
			UpdateLookAt();
		}
	
	}
		bool relative = mouse.rightButton || keyboard.LeftAlt && (mouse.leftButton || mouse.middleButton || mouse.rightButton);
		m_Mouse->SetMode(relative ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

		if(m_Keys.pressed.F)
		{
			FocusOnPosition(Vector3::Zero);
		}
}

void Game::CameraPan(Vector2 Delta)
{
	auto rect = m_deviceResources->GetOutputSize();
	float width = rect.right - rect.left;
	float height = rect.bottom - rect.top;

	const float x = std::min(width / 1000.f, 2.4f);
	const float xSpeed= .0366f * (x * x) - .1778f * x + .3021f;

	const float y = std::min(height / 1000.f, 2.4f);
	const float ySpeed = .0366f * (y * y) - .1778f * y + .3021f;

	m_LookPosition += GetCameraRightVector() * Delta.x * xSpeed * m_Distance;
	m_LookPosition += GetCameraUpVector() * Delta.y * ySpeed * m_Distance;

}

void Game::CameraOrbit(Vector2 Delta)
{
	float yawSign = GetCameraUpVector().y < 0 ? -1.f : 1.f;
	m_Yaw += yawSign * Delta.x * 12.f;
	m_Pitch -= Delta.y * 12.f;
}

void Game::CameraZoom(Vector2 Delta)
{
	float distance = m_Distance * 0.2f;
	distance = std::max(distance, 0.0f);
	float speed = distance * distance;
	speed = std::min(speed, 100.0f);
	
	m_Distance -= Delta.x * speed;
	if(m_Distance < 1.f)
	{
		m_LookPosition += GetCameraForwardVector();
		m_Distance = 1.f;
	}
	

}

void Game::UpdateLookAt()
{
	m_LookPosition = m_CameraPos + GetCameraForwardVector() * m_Distance;
}

void Game::FocusOnPosition(Vector3 NewLookAt)
{
	m_LookPosition = NewLookAt;
	m_Distance = 3.f;
}

Vector3 Game::CalculatePosition()
{
	m_CameraPos = m_LookPosition - GetCameraForwardVector() * m_Distance;
	return m_CameraPos;
}

bool Game::CanMoveCamera() const
{
	auto mouse = m_Mouse->GetState();
	return mouse.positionMode == Mouse::MODE_RELATIVE;
}

#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	 
    // TODO: Add your rendering code here.
    context;

	RenderSky();

	John::TransformCB transformConstants;
	Matrix Model = m_Mesh->GetTransformationMatrix();


	Matrix MVP = Model * m_ViewMatrix * m_ProjMatrix;
	transformConstants.MVP = MVP.Transpose();
	transformConstants.Model = Model.Transpose();

	context->UpdateSubresource(m_TransformCB.Get(), 0, nullptr, &transformConstants, 0, 0);

	John::ShadingCB shadingConstants;

	shadingConstants.LightPos = Vector3(1.f, 1.f, 2.f);
	shadingConstants.CameraPos = m_CameraPos;
	context->UpdateSubresource(m_ShadingCB.Get(), 0, nullptr, &shadingConstants, 0,0);

	ID3D11ShaderResourceView* const srvs[] =
	{
		m_EnvMap.SRV.Get(),
		m_IrradianceMap.SRV.Get(),
		m_BRDF_LUT.SRV.Get(),
		m_BrickAlbedo.SRV.Get(),
		m_BrickRoughness.SRV.Get(),
		m_BrickNormal.SRV.Get(),
		m_BrickMetallic.SRV.Get(),
	};

	ID3D11SamplerState* const states[] =
	{
		m_StandardSampler.Get(),
		m_BRDF_Sampler.Get()
	};

	context->VSSetConstantBuffers(0, 1, m_TransformCB.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, m_ShadingCB.GetAddressOf());
	context->PSSetShaderResources(0, _countof(srvs), srvs);
	context->PSSetSamplers(0, _countof(states),states);
	context->VSSetShader(m_PBRProgram.VertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_PBRProgram.PixelShader.Get(), nullptr, 0);
	context->IASetInputLayout(m_PBRProgram.InputLayout.Get());
	context->OMSetDepthStencilState(m_StandardDepthStencilState.Get(), 0);
	m_Mesh->Draw(context);

	RenderUI();

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

void Game::RenderUI()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

void Game::RenderSky()
{

	auto context = m_deviceResources->GetD3DDeviceContext();
	John::TransformCB transformConstants;

	Matrix Model = Matrix::Identity;
	XMMATRIX View = m_ViewMatrix;
	View.r[3] = g_XMIdentityR3;
	Matrix MVP = XMMatrixMultiply(View, m_ProjMatrix);
	transformConstants.MVP = MVP.Transpose();
	transformConstants.Model= Model;

	context->UpdateSubresource(m_TransformCB.Get(), 0, nullptr, &transformConstants, 0, 0);
	context->VSSetConstantBuffers(0, 1, m_TransformCB.GetAddressOf());
	context->OMSetDepthStencilState(m_SkyboxDepthStencilState.Get(), 0);
	context->IASetInputLayout(m_SkyboxProgram.InputLayout.Get());
	context->VSSetShader(m_SkyboxProgram.VertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_SkyboxProgram.PixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, m_IrradianceMap.SRV.GetAddressOf());
	context->PSSetSamplers(0, 1, m_StandardSampler.GetAddressOf());
	
	m_Skydome->Draw(context);


}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

Vector3 Game::GetCameraForwardVector() const
{
	Vector3 Forward = Vector3::Transform(Vector3::Forward, GetCameraOrientation());
	Forward.Normalize();
	return Forward;
}

Vector3 Game::GetCameraUpVector() const
{
	Vector3 Up = Vector3::Transform(Vector3::Up, GetCameraOrientation());
	Up.Normalize();
	return Up;
}

Vector3 Game::GetCameraRightVector() const
{
	Vector3 Right = Vector3::Transform(Vector3::Right, GetCameraOrientation());
	Right.Normalize();
	return Right;
}

Quaternion Game::GetCameraOrientation() const
{
	Quaternion q = Quaternion::CreateFromYawPitchRoll(XMConvertToRadians(m_Yaw),XMConvertToRadians( m_Pitch), 0.f);
	return q;
}

#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
	auto window = m_deviceResources->GetWindow();
	auto context = m_deviceResources->GetD3DDeviceContext();

	InitUI(window);
	InitializeSky();
	


	m_Mesh = John::LoadMeshFromFile("Content/sphere.obj");
	m_Mesh->Build(device);

	const std::vector<D3D11_INPUT_ELEMENT_DESC> meshInputLayout = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_StandardProgram = John::CreateShaderProgram(
		John::CompileShader(L"Shaders/PhongVS.hlsl", "main", "vs_5_0"),
		John::CompileShader(L"Shaders/PhongPS.hlsl", "main", "ps_5_0"),
		L"Shaders/PhongVS.hlsl",
		L"Shaders/PhongPS.hlsl",
		&meshInputLayout,
		device
		
	);


	m_PBRProgram = John::CreateShaderProgram(
		John::CompileShader(L"Shaders/PBRVS.hlsl", "main", "vs_5_0"),
		John::CompileShader(L"Shaders/PBRPS.hlsl", "main", "ps_5_0"),
		L"Shaders/PBRVS.hlsl",
		L"Shaders/PBRPS.hlsl",
		&meshInputLayout,
		device

	);


	m_SkyboxProgram = John::CreateShaderProgram(
		John::CompileShader(L"Shaders/SkyboxVS.hlsl", "main", "vs_5_0"),
		John::CompileShader(L"Shaders/SkyboxPS.hlsl", "main", "ps_5_0"),
		L"Shaders/SkyboxVS.hlsl",
		L"Shaders/SkyboxPS.hlsl",
		&meshInputLayout,
		device
	);


	m_TransformCB = John::CreateConstantBuffer<John::TransformCB>(device);
	m_ShadingCB = John::CreateConstantBuffer<John::ShadingCB>(device);

	m_StandardSampler = John::CreateSamplerState(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
	m_BrickAlbedo = John::CreateTexture(device, context, John::Image::FromFile("Content/Brick_Albedo.jpg"), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);
	m_BrickNormal = John::CreateTexture(device, context, John::Image::FromFile("Content/Brick_Normal.jpg"), DXGI_FORMAT_R8G8B8A8_UNORM, 1);
	m_BrickRoughness = John::CreateTexture(device, context, John::Image::FromFile("Content/Brick_Roughness.jpg"), DXGI_FORMAT_R8_UNORM, 1);
	m_BrickMetallic = John::CreateTexture(device, context, John::Image::FromFile("Content/Brick_Metallic.jpg"), DXGI_FORMAT_R8_UNORM, 1);

    // TODO: Initialize device dependent objects here (independent of window size).
    device;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	DX::ThrowIfFailed(
		device->CreateDepthStencilState(&depthStencilDesc, m_SkyboxDepthStencilState.ReleaseAndGetAddressOf())
	);

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	DX::ThrowIfFailed(
		device->CreateDepthStencilState(&depthStencilDesc, m_StandardDepthStencilState.ReleaseAndGetAddressOf())
	);

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
