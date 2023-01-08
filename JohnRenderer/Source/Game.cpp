//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Resources.h"
#include "JohnMesh.h"
#include "RenderObject.h"
#include "Material.h"
#include "Primitives.h"
#include "JohnPrimitive.h"
extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#ifdef _DEBUG
RENDERDOC_API_1_5_0* rdoc_api = NULL;
#endif

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
#ifdef _DEBUG
	// At init, on windows
	if ( HMODULE mod = GetModuleHandleA( "renderdoc.dll" ) )
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress( mod, "RENDERDOC_GetAPI" );
		int ret = RENDERDOC_GetAPI( eRENDERDOC_API_Version_1_5_0, (void **)&rdoc_api );
		assert( ret == 1 );
	}
#endif // DEBUG


    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources(); 
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	InitializeUI();

	m_Keyboard = std::make_unique<DirectX::Keyboard>();
	m_Mouse = std::make_unique<DirectX::Mouse>();
	m_Mouse->SetWindow( window );

	m_Camera = std::make_unique<Camera>();
	m_Camera->SetImageSize( width, height );
	m_Camera->SetPosition( Vector3( 0.f, 0.f, 3.f ) );
	m_Camera->SetRotation( Vector3( 0.f, XM_PI, 0.f) );

	m_CameraUserImpulseData = std::make_shared<CameraUserImpulseData>();

	m_LightPos = Vector3( .6f, 1.f, 2.f );


    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

void Game::InitializeDefaultAssets()
{
	auto device = m_deviceResources->GetD3DDevice ();


	John::ShaderProgram pbrProgram = John::CreateShaderProgram ( device, L"Shaders/PBRVS.hlsl", L"Shaders/PBRPS.hlsl" );
	m_Shaders.emplace( EShaderProgram::PBR, pbrProgram );

	std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>( device, m_StandardSampler.Get(), m_Shaders.find ( EShaderProgram::PBR)->second );

	m_Materials.push_back (defaultMaterial );

	std::shared_ptr<JohnMesh> MonkeyMesh = John::LoadMeshFromFile( "Content/sphere.obj" );
	MonkeyMesh->Build ( device );

	std::shared_ptr<RenderObject> SphereMesh = std::make_shared<RenderObject>();

	SphereMesh->SetMesh( MonkeyMesh );
	SphereMesh->SetMaterial ( m_Materials[0] );

	m_SourceMeshes.push_back( MonkeyMesh );

	m_Meshes.push_back ( SphereMesh );

	m_WorldGeo = std::make_shared<JohnMesh>();



	Vertex v1;
	v1.Position = Vector3( 0, 0, 1 );
	v1.Normal = Vector3( 0, 0, 1 );
	v1.TexCoord = Vector2( 0, 0 );

	Vertex v2;
	v2.Position = Vector3( 1, 0, -1 );
	v2.Normal = Vector3( 0, 0, 1 );
	v2.TexCoord = Vector2( .5, 1 );

	Vertex v3;
	v3.Position = Vector3( -1, 0, -1 );
	v3.Normal = Vector3( 0, 0, 1 );
	v3.TexCoord = Vector2( 1, 0 );

	m_WorldGeo->GetVertices ()->push_back ( v1 );
	m_WorldGeo->GetVertices ()->push_back ( v2 );
	m_WorldGeo->GetVertices ()->push_back ( v3 );

	m_WorldGeo->GetFaces ()->push_back ( { 1, 2, 3 } );
	m_WorldGeo->Build ( device );
}



void Game::InitializeUI()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	ImGui::StyleColorsDark();
	
	ImGui_ImplWin32_Init( m_deviceResources->GetWindow() );
	ImGui_ImplDX11_Init( m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext() );
}

void Game::InitializeSky( const char* EnvMapFile )
{
#ifdef _DEBUG
// 	if(rdoc_api)
// 	{
// 		rdoc_api->StartFrameCapture( NULL, NULL );
// 	}
#endif // DEBUG

	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_Environment = John::CreateEnvironmentFromFile( device, context, EnvMapFile );
	m_brdfSampler = John::CreateSamplerState( device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP );
	
	for(auto& material : m_Materials)
	{
		material->SetEnvironmentTextures ( m_Environment );
		material->SetBRDFSampler ( m_brdfSampler.Get() );
	}

	m_Sky = GeometricPrimitive::CreateGeoSphere( context, 2.f, 3.f, true );
	m_SkyEffect = std::make_unique<DX::SkyboxEffect>( device );

	m_Sky->CreateInputLayout( m_SkyEffect.get(), m_SkyInputLayout.ReleaseAndGetAddressOf() );

	m_SkyEffect->SetTexture( m_Environment.SpecularIBL.SRV.Get() );


// #ifdef _DEBUG
// 	if(rdoc_api)
// 	{
// 		rdoc_api->EndFrameCapture( NULL, NULL );
// 	}
//#endif // DEBUG


}

void Game::CleanSky()
{

}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
		TickUI();
    });

    Render();


	
}

void Game::TickUI()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			if(ImGui::MenuItem("Quit"))
			{
				PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Help"))
		{
			if(ImGui::MenuItem("Show ImGuiDemoWindow"))
			{
				m_bShowImGuiDemoWindow = !m_bShowImGuiDemoWindow;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if(m_bShowImGuiDemoWindow)
	{
	ImGui::ShowDemoWindow( &m_bShowImGuiDemoWindow );

	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
	ImGui::Begin( "Scene", NULL, window_flags );
	DrawSceneOutliner();

	ImGui::Separator();
	if(m_SelectedModel)
	{
		DrawModelDetails( m_SelectedModel );
	}
	ImGui::End();
	ImGui::Begin( "Editor" );
	ImGui::DragFloat3( "Light Position", &m_LightPos.x, .1f );
	ImGui::Text( "Camera" );
	if(ImGui::TreeNode( "Camera Settings" ))
	{
		ImGui::SliderFloat( "Mouse Sensitivity", &m_Camera->m_MovementSettings.MouseLookSensitivity, .0001f, .02f);
		ImGui::SliderFloat( "Orbit Speed", &m_Camera->m_MovementSettings.MouseOrbitSensitivity, 1.f, 10.f);
		ImGui::DragFloat( "Movement Speed", &m_Camera->m_MovementSettings.MovementSpeed, .1f, 0.f );
		bool bUsePhysicsBasedMovement = m_Camera->GetUsePhysicsBasedMovement();
		ImGui::Checkbox( "Use Physics Based Movement", &bUsePhysicsBasedMovement );
		m_Camera->SetUsePhysicsBasedMovement( bUsePhysicsBasedMovement );
		ImGui::TreePop();

	}



	
	ImGui::Text( "Camera Rotation: %f, %f", m_Camera->GetRotation().x, m_Camera->GetRotation().y );

	if ( ImGui::BeginMenu( "Switch Shader" ) )
	{

		ImGui::EndMenu();
	}

	ImGui::End();

	if(m_SelectedModel != nullptr)
	{
		TickGizmo();
	}
	static bool s_bRelativeLastFrame = false;

	if(!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&  m_MouseButtons.rightButton == Mouse::ButtonStateTracker::RELEASED && !m_bIsRelativeMode && m_DragAmount == Vector2::Zero)
	{

		ImGui::OpenPopup( "RightClick" );

	}
	if(m_SelectedModel != nullptr)
	{

	if(ImGui::BeginPopup("RightClick"))
	{
		if(ImGui::Button("Reset Transformations"))
		{
			m_SelectedModel->ResetTransformations();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	}
	s_bRelativeLastFrame = m_bIsRelativeMode;


}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

	PrepareInputState();

	Movement( elapsedTime );
	
	if(m_MouseButtons.leftButton == Mouse::ButtonStateTracker::PRESSED && !m_bIsRelativeMode)
	{
		SelectModel(MousePicking());
	}

	if(!m_bIsRelativeMode)
	{
		if ( m_Keys.pressed.Q )
		{
			DeselectAll();
		}
		else if(m_Keys.pressed.W)
		{
			m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		}
		else if ( m_Keys.pressed.E )
		{
			m_CurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
		}
		else if ( m_Keys.pressed.R )
		{
			m_CurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
		}
		if(m_Keys.pressed.Z)
		{
			m_CurrentGizmoMode = m_CurrentGizmoMode == ImGuizmo::MODE::WORLD ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
		}
	}
	m_MouseDeltaTracker.EndFrame();
    // TODO: Add your game logic here.
    elapsedTime;
}

void Game::Movement( float DeltaTime )
{
	
	if(m_KeyboardState.LeftShift && m_KeyboardState.LeftControl && m_KeyboardState.OemComma)
	{
		ReloadShaders();
	}



	m_CameraUserImpulseData->MoveForwardBackwardImpulse = 0.f;
	m_CameraUserImpulseData->MoveRightLeftImpulse = 0.f;
	m_CameraUserImpulseData->MoveUpDownImpulse = 0.f;
	m_CameraUserImpulseData->RotateYawImpulse = 0.f;
	m_CameraUserImpulseData->RotatePitchImpulse = 0.f;
	m_CameraUserImpulseData->RotateRollImpulse = 0.f;

	bool bForwardKeyState = false;
	bool bBackwardKeyState = false;
	bool bRightKeyState = false;
	bool bLeftKeyState = false;

	bool bUpKeyState = false;
	bool bDownKeyState = false;

	bForwardKeyState |= m_KeyboardState.W;
	bBackwardKeyState |= m_KeyboardState.S;
	bRightKeyState |= m_KeyboardState.D;
	bLeftKeyState |= m_KeyboardState.A;

	bUpKeyState |= m_KeyboardState.E;
	bDownKeyState |= m_KeyboardState.Q;

	if(bForwardKeyState)
	{
		m_CameraUserImpulseData->MoveForwardBackwardImpulse += 1.f;
	}
	if(bBackwardKeyState)
	{
		m_CameraUserImpulseData->MoveForwardBackwardImpulse -= 1.f;
	}
	if(bRightKeyState)
	{
		m_CameraUserImpulseData->MoveRightLeftImpulse += 1.;
	}
	if(bLeftKeyState)
	{
		m_CameraUserImpulseData->MoveRightLeftImpulse -= 1.f;
	}
	if(bUpKeyState)
	{
		m_CameraUserImpulseData->MoveUpDownImpulse += 1.f;
	}
	if(bDownKeyState)
	{
		m_CameraUserImpulseData->MoveUpDownImpulse -= 1.f;
	}

	Vector3 NewViewLocation = m_Camera->GetPosition();
	Vector3 NewViewEuler = m_Camera->GetRotationInDegrees();
	const float CameraSpeed = m_Camera->m_MovementSettings.MovementSpeed;
	float MovementDeltaUpperBound = 1.f;

	const float VelModRotSpeed = 900.f;
	const Vector3 RotEuler = m_Camera->GetRotationInDegrees();

	const float MouseSensitivity = m_Camera->m_MovementSettings.MouseLookSensitivity;
	m_CameraUserImpulseData->RotateRollVelocityModifier += VelModRotSpeed * RotEuler.x / MouseSensitivity;
	m_CameraUserImpulseData->RotatePitchVelocityModifier += VelModRotSpeed * RotEuler.y / MouseSensitivity;
	m_CameraUserImpulseData->RotateYawVelocityModifier += VelModRotSpeed * RotEuler.z / MouseSensitivity;

	float bHasMovement = false;

	if ( (*m_CameraUserImpulseData).RotateYawVelocityModifier != 0.0f ||
		(*m_CameraUserImpulseData).RotatePitchVelocityModifier != 0.0f ||
		(*m_CameraUserImpulseData).RotateRollVelocityModifier != 0.0f ||
		(*m_CameraUserImpulseData).MoveForwardBackwardImpulse != 0.0f ||
		(*m_CameraUserImpulseData).MoveRightLeftImpulse != 0.0f ||
		(*m_CameraUserImpulseData).MoveUpDownImpulse != 0.0f ||
		(*m_CameraUserImpulseData).RotateYawImpulse != 0.0f ||
		(*m_CameraUserImpulseData).RotatePitchImpulse != 0.0f ||
		(*m_CameraUserImpulseData).RotateRollImpulse != 0.0f
		)
	{
		bHasMovement = true;
	}

	int x, y;
	m_MouseDeltaTracker.GetMouseDelta( x, y );
	m_MouseDelta.x = (float)x;
	m_MouseDelta.y = (float)y;


	if ( m_MouseState.positionMode == Mouse::MODE_RELATIVE )
	{

	}



	if ( m_Keys.pressed.F )
	{
		m_Camera->FocusOnPosition( Vector3::Zero );
	}

	if (m_MouseState.positionMode == Mouse::MODE_RELATIVE)
	{
		if ( m_KeyboardState.LeftAlt && mouseDelta != Vector2::Zero)
		{
			Vector2 delta = mouseDelta * .00314f;
			bool move = false;
			if ( m_MouseState.middleButton )
			{
				m_Camera->MousePan( delta );
				move = true;
			}
			else if ( m_MouseState.leftButton )
			{
				m_Camera->MouseOrbit( delta );
				move = true;
			}
			else if ( m_MouseState.rightButton )
			{
				m_Camera->Mousezoom( delta );
				move = true;
			}
			m_bWasCameraMoved = move;

		}
		else
		{
			if ( m_MouseDelta != Vector2::Zero )
			{
				m_bWasCameraMoved = true;
			}
			else
			{
				m_bWasCameraMoved = false;
			}

			m_Camera->MoveAndRotateCamera( Vector3( 0, 0, 0 ), m_MouseDelta );

			m_Camera->UpdateSimulation( *m_CameraUserImpulseData,
				std::min( DeltaTime, MovementDeltaUpperBound ),
				CameraSpeed,
				NewViewLocation,
				NewViewEuler
			);

			if(m_MouseState.scrollWheelValue > 0.f)
			{
				m_Camera->AdjustSpeed( John::EScrollDirection::ScrollUp );
			}
			else if (m_MouseState.scrollWheelValue < 0.f)
			{
				m_Camera->AdjustSpeed( John::EScrollDirection::ScrollDown );
			}

			m_Mouse->ResetScrollWheelValue();
		}
	}

	m_bIsRelativeMode = m_MouseState.rightButton || m_KeyboardState.LeftAlt && (m_MouseState.leftButton || m_MouseState.middleButton || m_MouseState.rightButton);

	m_Mouse->SetMode( m_bIsRelativeMode ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE );




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

	DrawScene();
	
	DrawSky();


    m_deviceResources->PIXEndEvent();


	DrawToneMapping();
	RenderUI();

    // Show the new frame.
    m_deviceResources->Present();
}

void Game::RenderUI()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
}

void Game::DrawScene()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	John::PhongShadingCB shadingConstants;
	shadingConstants.CamPos = m_Camera->GetPosition();
	shadingConstants.LightPos = m_LightPos;

	context->UpdateSubresource( m_ShadingCB.Get(), 0, nullptr, &shadingConstants, 0, 0 );
	for(auto mesh : m_Meshes)
	{
		DrawMesh( mesh.get() );
	}
		m_WorldGeo->Draw ( context );

}

void Game::DrawSky()
{
	m_SkyEffect->SetView( m_Camera->GetViewMatrix());
	m_SkyEffect->SetProjection( m_Camera->GetProjectionMatrix() );
	m_Sky->Draw( m_SkyEffect.get(), m_SkyInputLayout.Get() );
}

void Game::DrawMesh( RenderObject* MeshToDraw )
{
	if(MeshToDraw == nullptr)
	{
		return;
	}
	auto context = m_deviceResources->GetD3DDeviceContext();

	John::PhongTransformCB transformConstants;

	Matrix model = MeshToDraw->GetTransformationMatrix();
	Matrix view = m_Camera->GetViewMatrix();
	Matrix proj = m_Camera->GetProjectionMatrix();
	Matrix MVP = model * view * proj;

	transformConstants.MVP = MVP.Transpose();
	transformConstants.Model = model.Transpose();

	context->UpdateSubresource( m_TransformCB.Get(), 0, nullptr, &transformConstants, 0, 0 );


	context->VSSetConstantBuffers( 0, 1, m_TransformCB.GetAddressOf() );
	context->PSSetConstantBuffers( 0, 1, m_ShadingCB.GetAddressOf() );


	MeshToDraw->Draw( context );

}

void Game::DrawToneMapping()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	context->OMSetRenderTargets( 1, &renderTarget, nullptr);
	context->IASetInputLayout( nullptr );
	context->VSSetShader( m_ToneMapProgram.VertexShader.Get(), nullptr, 0 );
	context->PSSetShader( m_ToneMapProgram.PixelShader.Get(), nullptr, 0 );
	context->PSSetShaderResources( 0, 1, m_DefaultFrameBuffer.SRV.GetAddressOf() );
	context->PSSetSamplers( 0, 1, m_ComputeSampler.GetAddressOf() );
	context->Draw( 3, 0 );


}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();


	ID3D11RenderTargetView* const nullRTV[] = { nullptr };
	ID3D11DepthStencilView* const nullDSV = nullptr;

	context->OMSetRenderTargets( _countof( nullRTV ), nullRTV, nullDSV );
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	context->PSSetShaderResources( 0, 2, null );
    context->ClearRenderTargetView(m_DefaultFrameBuffer.RTV.Get(), Colors::CornflowerBlue);
    context->ClearDepthStencilView(m_DefaultFrameBuffer.DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, m_DefaultFrameBuffer.RTV.GetAddressOf(), m_DefaultFrameBuffer.DSV.Get());

	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();
	context->ClearRenderTargetView( renderTarget, Colors::CornflowerBlue );
	context->ClearDepthStencilView( depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

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

void Game::OnMouseMove()
{
	//m_MouseDeltaTracker.UpdateState( m_Mouse->GetState());
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
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();
    // TODO: Initialize device dependent objects here (independent of window size).
    device;




	m_ToneMapProgram = John::CreateShaderProgram( device, L"Shaders/Tonemap.hlsl", L"Shaders/Tonemap.hlsl", "VS_Main", "PS_Main" );

	m_TransformCB = John::CreateConstantBuffer<John::PhongTransformCB>( device );
	m_ShadingCB = John::CreateConstantBuffer<John::PhongShadingCB>( device );

	m_BrickBaseColor = John::CreateTexture( device, context, Image::fromFile( "Content/Brick_Wall_BaseColor.jpg" ) , DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	context->GenerateMips( m_BrickBaseColor.SRV.Get() );
	m_BrickNormal = John::CreateTexture( device, context, Image::fromFile( "Content/Brick_Wall_Normal.jpg" ) , DXGI_FORMAT_R8G8B8A8_UNORM);
	m_BrickRoughness = John::CreateTexture( device, context, Image::fromFile( "Content/Brick_Wall_Roughness.jpg" ) , DXGI_FORMAT_R8G8B8A8_UNORM);


	m_StandardSampler = John::CreateSamplerState(device,D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
	
	m_ComputeSampler = John::CreateSamplerState(device,  D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP );

	InitializeDefaultAssets ();
	InitializeSky( "Content/environment.hdr" );
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.

	auto rect = m_deviceResources->GetOutputSize();

	if(m_Camera)
	{
		m_Camera->SetImageSize( rect.right, rect.bottom );

		m_SkyEffect->SetProjection( m_Camera->GetProjectionMatrix() );
	}

	m_DefaultFrameBuffer = John::CreateFrameBuffer( m_deviceResources->GetD3DDevice(), rect.right, rect.bottom, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT );

}

bool Game::CanMoveCamera() const
{
	return m_bIsRelativeMode;
}

void Game::ReloadShaders()
{
	for(auto& It : m_Shaders)
	{
		John::ShaderProgram* ProgramToUse = &It.second;
		ProgramToUse->VertexShader.Reset();
		ProgramToUse->PixelShader.Reset();
		ProgramToUse->InputLayout.Reset();

		*ProgramToUse = John::CreateShaderProgram( m_deviceResources->GetD3DDevice(), ProgramToUse->VertFileName, ProgramToUse->PixelFileName );

	}

	
}

void Game::AddPrimitive( John::EPrimitiveType type )
{
	std::shared_ptr<JohnPrimitive> newMesh;

	auto device = m_deviceResources->GetD3DDevice ();
	switch(type)
	{
	case John::EPrimitiveType::Sphere:
		newMesh = John::CreateSphere ( device, 3, 3 );
		newMesh->SetPrimitiveType ( type );
		break;
	}


	m_SourceMeshes.push_back ( newMesh );

	std::shared_ptr<RenderObject> newObject = std::make_shared<RenderObject>();
	newObject->SetMesh ( newMesh );
	newObject->SetMaterial ( m_Materials[0] );
	newObject->SetName ( "Sphere" );

	m_Meshes.push_back ( newObject );
}

void Game::TickGizmo()
{
	ImGuiIO& io = ImGui::GetIO();

	ImGuizmo::Enable( true );
	ImGuizmo::SetOrthographic( false );
	ImGuizmo::BeginFrame();

	ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
	Matrix view = m_Camera->GetViewMatrix();
	Matrix proj = m_Camera->GetProjectionMatrix();
	Matrix model = m_SelectedModel->GetTransformationMatrix();

	ImGuizmo::SetID( 0 );
	bool bManipulated = ImGuizmo::Manipulate( *view.m, *proj.m, m_CurrentGizmoOperation, m_CurrentGizmoMode, *model.m ) && !m_bIsRelativeMode;

	if(bManipulated)
	{
		Vector3 NewTrans;
		Quaternion NewRot;
		Vector3 NewScale;

		model.Decompose( NewScale, NewRot, NewTrans );
		John::Transform NewTransform;
		NewTransform.Position = NewTrans;
		NewTransform.Rotation = NewRot;
		NewTransform.Scale = NewScale;

		m_SelectedModel->SetTransform( NewTransform );
	}
}

void Game::DrawSceneOutliner()
{
	ImGui::Text( "Outliner" );
	if(ImGui::BeginMenuBar())
	{
		if(ImGui::BeginMenu("Add"))
		{
			if(ImGui::MenuItem("Mesh"))
			{

			}
			if(ImGui::MenuItem("Sphere"))
			{
				AddPrimitive ( John::EPrimitiveType::Sphere );


			}
			ImGui::EndMenu();
		}
		if(ImGui::Button("Change Sky"))
		{

		}
		ImGui::EndMenuBar();
	}

	if(ImGui::BeginTable("Split", 1, ImGuiTableFlags_BordersOuter))
	{

		int uid = 0;
		for ( auto& mesh : m_Meshes )
		{

			DrawModelInOutliner( "Object: ", uid , mesh);
			uid++;

		}


		ImGui::EndTable();
	
	}

}

void Game::DrawModelInOutliner( const char* prefix, int uid, std::shared_ptr<RenderObject> mesh )
{
	ImGui::PushID( uid );
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex( 0 );
	ImGui::AlignTextToFramePadding();
	bool node_open = ImGui::TreeNode( mesh->GetName().c_str() );
	/*	ImGui::TableSetColumnIndex( 1 );*/


	if ( ImGui::IsItemClicked() )
	{
		SelectModel( mesh.get() );
	}

	bool bEntityDeleted = false;
	if ( ImGui::BeginPopupContextItem() )
	{
		if ( ImGui::MenuItem( "DeleteActor" ) )
		{
			bEntityDeleted = true;
		}
		if ( ImGui::MenuItem( "Reset Transform" ) )
		{
			if ( m_SelectedModel )
			{
				m_SelectedModel->ResetTransformations();
			}

		}

		ImGui::EndPopup();
	}

	if ( node_open )
	{
		ImGui::TreePop();
	}
	ImGui::PopID();

	if ( bEntityDeleted )
	{
		if ( m_SelectedModel )
		{
			DeleteMesh( mesh );
			DeselectAll();
		}
	}
}

void Game::DrawModelDetails(RenderObject* Mesh)
{
	Vector3 ModelTrans = Mesh->GetPosition();
	Vector3 ModelRot = Mesh->GetRotationEuler();
	Vector3 ModelScale = Mesh->GetScale();

	if(ImGui::DragFloat3("Translation", &ModelTrans.x, .1f))
	{
		Mesh->SetPosition( ModelTrans );
	}
	if ( ImGui::DragFloat3( "Rotation", &ModelRot.x, .1f ) )
	{
		Mesh->SetRotationEuler( ModelRot);
	}
	if ( ImGui::DragFloat3( "Scale", &ModelScale.x, .1f ) )
	{
		Mesh->SetScale( ModelScale);
	}

	JohnPrimitive* prim = static_cast<JohnPrimitive*>(Mesh->GetMesh ().get());

	if(prim)
	{
		auto device = m_deviceResources->GetD3DDevice ();
		float primSize = float(prim->GetSize ());
		int tess = prim->GetTessellation ();
		if(ImGui::DragFloat("Primitive Size", &primSize, .1f))
		{
			prim->SetSize ( primSize );
			prim->Build ( device );
		}
		if(ImGui::DragInt ("Tessellation", &tess, 1.f, 3, 7))
		{
			prim->SetTessellation ( tess );
			prim->Build ( device );
		}
	}
}

void Game::DeleteMesh( std::shared_ptr<RenderObject> MeshToDelete )
{
	m_Meshes.erase( std::remove( m_Meshes.begin(), m_Meshes.end(), MeshToDelete ) );
}



void Game::ConvertMovementDeltaToDragRot(Vector3& InOutDragDelta,  Vector3& OutDrag, Vector3& Rot  )
{
	if(m_MouseState.rightButton && !m_MouseState.leftButton)
	{
		Rot.y = InOutDragDelta.x * m_Camera->m_MovementSettings.MouseLookSensitivity;
		Rot.x = InOutDragDelta.y * m_Camera->m_MovementSettings.MouseLookSensitivity;

	}
}

void Game::SelectModel( JohnMesh* ModelToSelect )
{
	if(ModelToSelect != nullptr)
	{
		m_SelectedModel = ModelToSelect;
	}
}

void Game::PrepareInputState()
{
	m_MouseState = m_Mouse->GetState();
	m_KeyboardState = m_Keyboard->GetState();
	m_Keys.Update( m_KeyboardState );
	m_MouseButtons.Update( m_MouseState );
}

void Game::DeselectAll()
{
	m_SelectedModel = nullptr;
}

RenderObject* Game::MousePicking()
{
	if(m_Meshes.size() == 0)
	{
		return nullptr;
	}


	uint32_t x, y;
	x = m_MouseState.x;
	y = m_MouseState.y;
	

	auto size = m_deviceResources->GetOutputSize();
	float width = size.right;
	float height = size.bottom;

	Matrix proj = m_Camera->GetProjectionMatrix();
	Matrix view = m_Camera->GetViewMatrix();
	Matrix inverseView = view.Invert();

	
	float pointX = ((2.f * (float)x) / width) - 1.f;
	float pointY = (((2.f * (float)y) / height) - 1.f) * -1.f;
	
	Vector3 rayOrigViewSpace( 0.f, 0.f, 0.f );
	Vector3 rayDirViewSpace( pointX, pointY, 1.f );

	Vector3 rayOrigWorldSpace = Vector3::Transform( rayOrigViewSpace, inverseView );
	Vector3 rayDirWorldSpace = Vector3::TransformNormal( rayDirViewSpace, inverseView );

	rayDirWorldSpace.Normalize();

	float hitDistance = FLT_MAX;

	bool bResult = false;
	RenderObject* ActorToSelect = nullptr;

	for(auto& renderObject : m_Meshes)
	{
		auto mesh = renderObject->GetMesh ();
		Ray ray;
		ray.direction = rayDirWorldSpace;
		ray.position = rayOrigWorldSpace;
		auto vertices = *mesh->GetVertices();
		for(auto face : *mesh->GetFaces())
		{
			uint32_t Index1 = face.v1;
			uint32_t Index2 = face.v2;
			uint32_t Index3 = face.v3;

			Vertex Vertex1 = vertices[Index1];
			Vertex Vertex2 = vertices[Index2];
			Vertex Vertex3 = vertices[Index3];

			float dist = 100.f;
			bResult = ray.Intersects( Vertex1.Position, Vertex2.Position, Vertex3.Position, dist );

			if(bResult)
			{
				hitDistance = std::min( hitDistance, dist );
				ActorToSelect = renderObject.get();
				break;
				
			}
			
			


		}

	}
	if(bResult)
	{
		return ActorToSelect;
	}
	else
	{
		return nullptr;
	}
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
