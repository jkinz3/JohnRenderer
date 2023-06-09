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
#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "AssetManager.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#ifdef _DEBUG
RENDERDOC_API_1_5_0* rdoc_api = NULL;
#endif

Game::Game() noexcept(false)
{
    
	
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    DX::DeviceResources::Get().RegisterDeviceNotify(this);

	NullEntity.SetEntityHandle(entt::null);

	m_ViewportRenderTarget = std::make_unique<DX::RenderTexture>( DXGI_FORMAT_R16G16B16A16_FLOAT );
	m_FinalRenderTarget = std::make_unique<DX::RenderTexture>( DXGI_FORMAT_R16G16B16A16_FLOAT);

	m_ViewportWindowName = std::string("Viewport");

}

Game::~Game()
{
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


    DX::DeviceResources::Get().SetWindow(window, width, height);

    DX::DeviceResources::Get().CreateDeviceResources(); 
    CreateDeviceDependentResources();

	m_Camera = std::make_unique<Camera>();
	m_Camera->SetImageSize(width, height);
	m_Camera->SetPosition(Vector3(0.f, 0.f, 3.f));
	m_Camera->SetRotation(Vector3(0.f, 180.f, 0.f));

    DX::DeviceResources::Get().CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	InitializeUI();

	m_Keyboard = std::make_unique<DirectX::Keyboard>();
	m_Mouse = std::make_unique<DirectX::Mouse>();
	m_Mouse->SetWindow( window );



	m_CameraUserImpulseData = std::make_shared<CameraUserImpulseData>();


	m_bRightClickOpen = false;
	m_bWantsRightClick = false;

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

void Game::InitializeDefaultAssets()
{
	auto device = DX::DeviceResources::Get().GetD3DDevice ();


	John::ShaderProgram pbrProgram = John::CreateShaderProgram (  L"Shaders/PBRVS.hlsl", L"Shaders/PBRPS.hlsl" );
	m_Shaders.emplace( EShaderProgram::PBR, pbrProgram );
	std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>(  m_StandardSampler.Get(), EShaderProgram::PBR);
	AssetManager::Get().RegisterMaterial (defaultMaterial);

	John::ShaderProgram lightShereProgram = John::CreateShaderProgram(  L"Shaders/SimpleVS.hlsl", L"Shaders/LightSpherePS.hlsl" );
	m_Shaders.emplace( EShaderProgram::LightSphere, lightShereProgram );
	std::shared_ptr<Material> lightSphereMaterial = std::make_shared<Material>(  m_StandardSampler.Get(),  EShaderProgram::LightSphere );
	AssetManager::Get().RegisterMaterial( lightSphereMaterial );

	std::shared_ptr<JohnMesh> MonkeyMesh = John::LoadMeshFromFile( "Content/sphere.obj" );

	AssetManager::Get().RegisterMesh(MonkeyMesh);
	MonkeyMesh->Build ( device );

	Entity entity = m_Scene->CreateEntity("DefaultSphere");
	entity.AddComponent<MeshComponent>();
	entity.AddComponent<TransformComponent>();
	entity.AddComponent<NameComponent>();
	entity.GetComponent<MeshComponent>().Mesh = MonkeyMesh;
	entity.GetComponent<MeshComponent>().Material = defaultMaterial;
	entity.GetComponent<NameComponent>().Name = std::string("DefaultSphere");

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

}



void Game::InitializeUI()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();

	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.f;
		style.Colors[ImGuiCol_WindowBg].w = 1.f;
	}


	ImGui::StyleColorsDark();
	
	ImGui_ImplWin32_Init( DX::DeviceResources::Get().GetWindow() );
	ImGui_ImplDX11_Init( DX::DeviceResources::Get().GetD3DDevice(), DX::DeviceResources::Get().GetD3DDeviceContext() );
}

void Game::InitializeSky( const char* EnvMapFile )
{
#ifdef _DEBUG
// 	if(rdoc_api)
// 	{
// 		rdoc_api->StartFrameCapture( NULL, NULL );
// 	}
#endif // DEBUG

	auto device = DX::DeviceResources::Get().GetD3DDevice();
	auto context = DX::DeviceResources::Get().GetD3DDeviceContext();
	m_Environment = John::CreateEnvironmentFromFile( EnvMapFile );
	m_brdfSampler = John::CreateSamplerState(  D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP );

	AssetManager::Get().IterateOverMaterials ([&](Material* material)
		{
			material->SetEnvironmentTextures (m_Environment);
			material->SetBRDFSampler (m_brdfSampler.Get());
		}
	);

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
	ImGuizmo::BeginFrame();
	ImGuiDockNodeFlags NodeFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpaceOverViewport ( ImGui::GetMainViewport (), NodeFlags );
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
	ImGui::Text( "Camera" );
	if(ImGui::TreeNode( "Camera Settings" ))
	{
		ImGui::DragFloat( "Mouse Sensitivity", &m_Camera->m_MovementSettings.MouseLookSensitivity, .0001f);
		ImGui::DragFloat( "Orbit Speed", &m_Camera->m_MovementSettings.MouseOrbitSensitivity, 1.f);
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


	static bool s_bRelativeLastFrame = false;


	if(m_SelectedModel.GetEntityHandle () != entt::null)
	{

// 	if(ImGui::BeginPopup("RightClick"))
// 	{
// 		if(ImGui::Button("Reset Transformations"))
// 		{
// 		//	m_SelectedModel->ResetTransformations();
// 			ImGui::CloseCurrentPopup();
// 			m_bWantsRightClick = false;
// 		}
// 		ImGui::EndPopup();
// 	}


	}
	DrawCameraViewport ();

}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	float elapsedTime = float(timer.GetElapsedSeconds());

	PrepareInputState();

	Movement(elapsedTime);

	if ( m_MouseButtons.leftButton == Mouse::ButtonStateTracker::PRESSED && !m_bIsRelativeMode && m_bViewportPanelHovered && !m_bIsGizmoHovered)
	{
		SelectModel(MousePicking());
	}

	if(m_Keys.pressed.Delete)
	{
		DeleteMesh (m_SelectedModel);
		DeselectAll ();
	}

	bool bAreAnyCameraMovementKeysPressed = m_MouseButtons.rightButton == Mouse::ButtonStateTracker::PRESSED ||
		(
			m_KeyboardState.LeftAlt && (m_MouseButtons.leftButton == Mouse::ButtonStateTracker::PRESSED || m_MouseButtons.middleButton == Mouse::ButtonStateTracker::PRESSED
				|| m_MouseButtons.rightButton == Mouse::ButtonStateTracker::PRESSED)
		);

	if(bAreAnyCameraMovementKeysPressed && IsCameraViewportHovered ())
	{
		m_bCanMoveCamera = true;
	}
	else if (!m_bIsRelativeMode)
	{
		m_bCanMoveCamera = false;
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
	
	if(m_MouseState.rightButton)
	{

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
	}

	Vector3 NewViewLocation = m_Camera->GetPosition();
	Vector3 NewViewEuler = m_Camera->GetRotation ();
	const float CameraSpeed = m_Camera->m_MovementSettings.MovementSpeed;
	float MovementDeltaUpperBound = 1.f;

	const float VelModRotSpeed = 900.f;
	const Vector3 RotEuler = m_Camera->GetRotation();

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



	m_bWasCameraMoved = false;

	m_Camera->UpdateSimulation(*m_CameraUserImpulseData,
		std::min(DeltaTime, MovementDeltaUpperBound),
		CameraSpeed,
		NewViewLocation,
		NewViewEuler
	);

	if ( m_Keys.pressed.F )
	{
		if(m_SelectedModel != NullEntity)
		{
			auto transformComp = m_SelectedModel.GetComponent<TransformComponent>();
			m_Camera->FocusOnPosition (transformComp.GetTranslation());
		}
		else
		{
			m_Camera->FocusOnPosition( Vector3::Zero );

		}
	}

	if (m_MouseState.positionMode == Mouse::MODE_RELATIVE && m_bCanMoveCamera)
	{

		if ( m_KeyboardState.LeftAlt && m_MouseDelta != Vector2::Zero)
		{
			Vector2 delta = m_MouseDelta * .00314f;
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
		else if (m_MouseState.rightButton)
		{
			if ( m_MouseDelta != Vector2::Zero )
			{
				m_bWasCameraMoved = true;
			}

			m_Camera->MoveAndRotateCamera( Vector3( 0, 0, 0 ), m_MouseDelta );



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
		else if (m_MouseButtons.rightButton == Mouse::ButtonStateTracker::RELEASED && m_bWasCameraMoved )
		{
			m_bWasCameraMoved = false;
			m_bJustFinishedMovingCamera = true;
		}
		else
		{
			m_bJustFinishedMovingCamera = false;
		}
	}
	if ( m_bIsRelativeMode )
	{
		m_RelativeMouseDelta += m_MouseDelta;
	}
	m_bIsRelativeMode = (m_MouseState.rightButton || m_KeyboardState.LeftAlt && (m_MouseState.leftButton || m_MouseState.middleButton || m_MouseState.rightButton));

	m_Mouse->SetMode( m_bIsRelativeMode && m_bCanMoveCamera ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE );

	SetInputEnabled (!m_bIsRelativeMode);

	if(m_MouseButtons.rightButton == GamePad::ButtonStateTracker::RELEASED)
	{
		if(m_RelativeMouseDelta == Vector2::Zero || m_bRightClickOpen)
		{
			m_bWantsRightClick = true;
			
		}
		else
		{
			m_RelativeMouseDelta = Vector2::Zero;
		}
	}



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

    DX::DeviceResources::Get().PIXBeginEvent(L"Render");
    auto context = DX::DeviceResources::Get().GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    context;
	

	DrawScene();
	
	DrawSky();

	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	context->PSSetShaderResources( 0, _countof( nullSRV ), nullSRV );
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	context->OMSetRenderTargets( 1, nullViews, nullptr );

	DrawToneMapping();


	RenderUI();

    // Show the new frame.
    DX::DeviceResources::Get().Present();
}

void Game::RenderUI()
{
	auto renderTarget = DX::DeviceResources::Get().GetRenderTargetView ();
	auto context = DX::DeviceResources::Get().GetD3DDeviceContext ();

	context->OMSetRenderTargets( 1, &renderTarget, nullptr );
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows ();
		ImGui::RenderPlatformWindowsDefault ();
	}
}

void Game::DrawCameraViewport()
{
	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2(0,0));
	ImGui::Begin( m_ViewportWindowName.c_str());

	size_t width, height;
	m_ViewportRenderTarget->GetTextureSize ( width, height );
	m_bViewportPanelHovered = ImGui::IsWindowHovered ();
	size_t regionWidth, regionHeight;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail ();
	regionWidth = viewportPanelSize.x;
	regionHeight = viewportPanelSize.y;
	m_ViewportSize = Vector2(viewportPanelSize.x, viewportPanelSize.y);
	auto windowSize = ImGui::GetWindowSize ();
	ImVec2 minBound = ImGui::GetWindowPos (); 
	auto viewportOffset = ImGui::GetCursorPos ();
	minBound.x -= viewportOffset.x;
	minBound.y -= viewportOffset.y;
	ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
	m_ViewportBounds[0] = { minBound.x, minBound.y };
	m_ViewportBounds[1] = { maxBound.x, maxBound.y };

	
	if(width != regionWidth || height != regionHeight)
	{
		m_ViewportRenderTarget->SizeResources ( regionWidth, regionHeight );
		m_FinalRenderTarget->SizeResources ( regionWidth, regionHeight );
		
		m_Camera->SetImageSize ( regionWidth, regionHeight );
	}
	ImGui::Image( (void*)m_FinalRenderTarget->GetShaderResourceView (), ImVec2(regionWidth, regionHeight) );
	if ( m_SelectedModel.GetEntityHandle () != entt::null )
	{
		TickGizmo();
	}
	ImGui::PopStyleVar();
	ImGui::End();
}

void Game::DrawScene()
{
	DX::DeviceResources::Get().PIXBeginEvent (L"Draw Scene");
	auto context = DX::DeviceResources::Get().GetD3DDeviceContext();

	auto renderTarget = m_ViewportRenderTarget->GetRenderTargetView ();
	auto depthTarget = m_ViewportRenderTarget->GetDepthStencilView ();
	
	size_t width, height;
	m_ViewportRenderTarget->GetTextureSize ( width, height );
	CD3D11_VIEWPORT viewport(
		0.f,
		0.f,
		width,
		height
	);

	context->OMSetRenderTargets( 1, &renderTarget, depthTarget );
	context->ClearRenderTargetView( renderTarget, Colors::SlateGray );
	context->ClearDepthStencilView( depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
	context->RSSetViewports( 1, &viewport );

	auto lights = m_Scene->m_Registry.view<PointLightComponent, TransformComponent>();

	John::PhongShadingCB shadingConstants = {};
	int index = 0;
	for(auto  entity: lights )
	{

		if(index >= (MaxPointLights))
		{
			break;
		}
		auto [light, transform] = lights.get<PointLightComponent, TransformComponent>( entity );

		Light l;
		l.LightPos = transform.GetTranslation();
		l.LightColor = light.LightColor;
		l.LightIntensity = light.LightIntensity;

		shadingConstants.Lights[index] = l;
		index++;
	}


	shadingConstants.CamPos = m_Camera->GetPosition();

	context->UpdateSubresource( m_ShadingCB.Get(), 0, nullptr, &shadingConstants, 0, 0 );
	Matrix view = m_Camera->GetViewMatrix();
	Matrix proj = m_Camera->GetProjectionMatrix();




	auto group = m_Scene->GetAllEntitiesWith<TransformComponent, MeshComponent>();
	for(auto e : group)
	{
		Entity entity = { e, m_Scene.get() };
		
		MeshComponent& meshComponent = entity.GetComponent<MeshComponent>();
		TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();


		Matrix model = transformComponent.GetTransformationMatrix();
		Matrix MVP = model * view * proj;

		if(entity.HasComponent<PointLightComponent>())
		{
			John::LightSphereTransformCB transformConstants;

			meshComponent.Material->Apply( context );
			transformConstants.MVP = MVP.Transpose();
			transformConstants.Model = model.Transpose();

			context->UpdateSubresource( m_LightSphereTransformCB.Get(), 0, nullptr, &transformConstants, 0, 0 );



			context->VSSetConstantBuffers( 0, 1, m_LightSphereTransformCB.GetAddressOf() );

		}
		else
		{
			John::PhongTransformCB transformConstants;

			meshComponent.Material->Apply( context );

			Matrix normal = model.Invert ();

			transformConstants.MVP = MVP.Transpose();
			transformConstants.Model = model.Transpose();
			transformConstants.Normal = normal;

			context->UpdateSubresource( m_TransformCB.Get(), 0, nullptr, &transformConstants, 0, 0 );


			context->VSSetConstantBuffers( 0, 1, m_TransformCB.GetAddressOf() );
			context->PSSetConstantBuffers( 0, 1, m_ShadingCB.GetAddressOf() );

		}
		EShaderProgram shaderType = meshComponent.Material->GetShaderProgram();
		auto it = m_Shaders.find( shaderType );
		if ( it != m_Shaders.end() )
		{
			John::ShaderProgram shaderProgram = it->second;

			context->VSSetShader( shaderProgram.VertexShader.Get(), nullptr, 0 );
			context->PSSetShader( shaderProgram.PixelShader.Get(), nullptr, 0 );
			context->IASetInputLayout( shaderProgram.InputLayout.Get() );
		}
			meshComponent.Mesh->Draw( context );
	}

	for(auto mesh : m_RenderObjects)
	{
		DrawMesh( mesh.get() );
	}

	DX::DeviceResources::Get().PIXEndEvent ();

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
	auto context = DX::DeviceResources::Get().GetD3DDeviceContext();



}

void Game::DrawToneMapping()
{
	auto context = DX::DeviceResources::Get().GetD3DDeviceContext();
	auto renderTarget = m_FinalRenderTarget->GetRenderTargetView();
	auto SRV = m_ViewportRenderTarget->GetShaderResourceView ();
	context->OMSetRenderTargets( 1, &renderTarget, nullptr);
	context->IASetInputLayout( nullptr );
	context->VSSetShader( m_ToneMapProgram.VertexShader.Get(), nullptr, 0 );
	context->PSSetShader( m_ToneMapProgram.PixelShader.Get(), nullptr, 0 );
	context->PSSetShaderResources( 0, 1,  &SRV);
	context->PSSetSamplers( 0, 1, m_ComputeSampler.GetAddressOf() );
	context->Draw( 3, 0 );


}

// Helper method to clear the back buffers.
void Game::Clear()
{
    DX::DeviceResources::Get().PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = DX::DeviceResources::Get().GetD3DDeviceContext();
	auto renderTarget = DX::DeviceResources::Get().GetRenderTargetView ();
	auto depthTarget = DX::DeviceResources::Get().GetDepthStencilView ();

	ID3D11RenderTargetView* const nullRTV[] = { nullptr };
	ID3D11DepthStencilView* const nullDSV = nullptr;

	context->OMSetRenderTargets( _countof( nullRTV ), nullRTV, nullDSV );
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	context->PSSetShaderResources( 0, 2, null );
    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthTarget);

    // Set the viewport.
    auto const viewport = DX::DeviceResources::Get().GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    DX::DeviceResources::Get().PIXEndEvent();
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
    auto const r = DX::DeviceResources::Get().GetOutputSize();
    DX::DeviceResources::Get().WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    DX::DeviceResources::Get().UpdateColorSpace();
}

void Game::OnMouseMove()
{
	m_MouseDeltaTracker.UpdateState( m_Mouse->GetState());
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!DX::DeviceResources::Get().WindowSizeChanged(width, height))
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

void Game::SetInputEnabled(bool bEnabled)
{
	ImGuiIO& io = ImGui::GetIO();

	if(bEnabled)
	{
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
	}
	else
	{
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	}
}

#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = DX::DeviceResources::Get().GetD3DDevice();
	auto context = DX::DeviceResources::Get().GetD3DDeviceContext();
    // TODO: Initialize device dependent objects here (independent of window size).
    device;

	m_Scene = std::make_shared<Scene>();


	m_ToneMapProgram = John::CreateShaderProgram(  L"Shaders/Tonemap.hlsl", L"Shaders/Tonemap.hlsl", "VS_Main", "PS_Main" );

	m_TransformCB = John::CreateConstantBuffer<John::PhongTransformCB>(  );
	m_ShadingCB = John::CreateConstantBuffer<John::PhongShadingCB>(  );

	m_LightSphereTransformCB = John::CreateConstantBuffer<John::LightSphereTransformCB>(  );

	m_BrickBaseColor = John::CreateTexture( Image::fromFile( "Content/Brick_Wall_BaseColor.jpg" ) , DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	context->GenerateMips( m_BrickBaseColor.SRV.Get() );
	m_BrickNormal = John::CreateTexture(   Image::fromFile( "Content/Brick_Wall_Normal.jpg" ) , DXGI_FORMAT_R8G8B8A8_UNORM);
	m_BrickRoughness = John::CreateTexture(   Image::fromFile( "Content/Brick_Wall_Roughness.jpg" ) , DXGI_FORMAT_R8G8B8A8_UNORM);


	m_StandardSampler = John::CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
	
	m_ComputeSampler = John::CreateSamplerState( D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP );

	InitializeDefaultAssets ();
	InitializeSky( "Content/environment.hdr" );

	m_ViewportRenderTarget->SetDevice (  device);
	m_FinalRenderTarget->SetDevice (  device);


}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.

	auto rect = DX::DeviceResources::Get().GetOutputSize();


	

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

		*ProgramToUse = John::CreateShaderProgram(  ProgramToUse->VertFileName, ProgramToUse->PixelFileName );

	}

	
}

void Game::ImportMeshFromFile(const char* File)
{
	auto device = DX::DeviceResources::Get().GetD3DDevice ();
	std::shared_ptr<JohnMesh> mesh = John::LoadMeshFromFile(File);

	AssetManager::Get().RegisterMesh(mesh);
	mesh->Build (device);

	Entity entity = m_Scene->CreateEntity(File);
	entity.AddComponent<MeshComponent>();
	entity.AddComponent<TransformComponent>();
	entity.GetComponent<MeshComponent>().Mesh = mesh;
	entity.GetComponent<MeshComponent>().Material = AssetManager::Get().GetDefaultMaterial();

	SelectModel( entity );
}

void Game::AddPrimitive( John::EPrimitiveType type )
{
	std::shared_ptr<JohnPrimitive> newMesh;
	std::string objectName;
	auto device = DX::DeviceResources::Get().GetD3DDevice ();
	switch(type)
	{
	case John::EPrimitiveType::Sphere:
		newMesh = John::CreateSphere ( 3, 3 );
		newMesh->SetPrimitiveType ( type );
		objectName = std::string( "Sphere" );
		break;
	case John::EPrimitiveType::Cube:
		newMesh = John::CreateCube(  3 );
		newMesh->SetPrimitiveType( type );
		objectName = std::string( "Cube" );
		break;
	case John::EPrimitiveType::Torus:
		newMesh = John::CreateTorus( 1.f, .33f, 32 );
		newMesh->SetPrimitiveType(type);
		objectName = std::string("Torus");
		break;

	}


	AssetManager::Get().RegisterMesh (newMesh);
	Entity entity = m_Scene->CreateEntity( objectName.c_str() );
	entity.AddComponent<MeshComponent>();
	entity.AddComponent<TransformComponent>();
	entity.AddComponent<NameComponent> ();
	entity.GetComponent<MeshComponent>().Mesh = newMesh;
	entity.GetComponent<MeshComponent>().Material = AssetManager::Get().GetDefaultMaterial();
	entity.GetComponent<NameComponent>().Name = objectName;

	SelectModel( entity );
}

void Game::AddPointLight()
{
	auto device = DX::DeviceResources::Get().GetD3DDevice();
	std::shared_ptr<JohnPrimitive> lightMesh;
	lightMesh = John::CreateSphere( .2f, 6 );
	AssetManager::Get().RegisterMesh( lightMesh );
	Entity entity = m_Scene->CreateEntity( "PointLight" );
	entity.AddComponent<NameComponent>();
	entity.AddComponent<PointLightComponent>();
	entity.AddComponent<TransformComponent>();
	entity.AddComponent<MeshComponent>();

	entity.GetComponent<PointLightComponent>().LightColor = Vector3( 1.f, 1.f, 1.f );
	entity.GetComponent<PointLightComponent>().LightIntensity = 1.f;
	entity.GetComponent<MeshComponent>().Mesh = lightMesh;
	entity.GetComponent<MeshComponent>().Material = AssetManager::Get().GetLightSphereMaterial();
	entity.GetComponent<NameComponent>().Name = std::string("PointLight");

	SelectModel( entity );


}

void Game::TickGizmo()
{
	ImGuiIO& io = ImGui::GetIO();

	ImGuizmo::Enable( true );
	ImGuizmo::SetOrthographic( false );

	ImGuizmo::SetDrawlist();

	float rw = (float)ImGui::GetWindowWidth ();
	float rh = (float)ImGui::GetWindowHeight ();

	ImGuizmo::SetRect(ImGui::GetWindowPos ().x, ImGui::GetWindowPos ().y, rw, rh);
	Matrix view = m_Camera->GetViewMatrix();
	Matrix proj = m_Camera->GetProjectionMatrix();

	TransformComponent& transComp = m_SelectedModel.GetComponent<TransformComponent>();
	Matrix model = transComp.GetTransformationMatrix();

	ImGuizmo::SetID( 0 );
	bool bManipulated = ImGuizmo::Manipulate( *view.m, *proj.m, m_CurrentGizmoOperation, m_CurrentGizmoMode, *model.m ) && !m_bIsRelativeMode;
	m_bIsGizmoHovered = ImGuizmo::IsOver();

	if(bManipulated)
	{
		Vector3 NewTrans;
		Quaternion NewRot;
		Vector3 NewScale;
		model.Decompose( NewScale, NewRot, NewTrans );
		TransformComponent NewTransform;
		transComp.SetTranslation(NewTrans);
		//ctor3 RotEuler = NewRot.ToEuler ();
		transComp.SetRotation( NewRot);
		transComp.SetScale( NewScale);

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
				ImportMesh();

			}
			if(ImGui::MenuItem("Sphere"))
			{
				AddPrimitive ( John::EPrimitiveType::Sphere );


			}
			if(ImGui::MenuItem("Torus"))
			{
				AddPrimitive( John::EPrimitiveType::Torus );
			}
			if(ImGui::MenuItem("Cube"))
			{
				AddPrimitive( John::EPrimitiveType::Cube );
			}
			if(ImGui::MenuItem("PointLight"))
			{
				AddPointLight();


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
		auto ents = m_Scene->GetAllEntitiesWith < MeshComponent>();
		for ( auto mesh : ents )
		{
			Entity entity = { mesh, m_Scene.get() };
			DrawModelInOutliner( "Object: ", uid , entity);
			uid++;

		}


		ImGui::EndTable();
	
	}

}

void Game::DrawModelInOutliner( const char* prefix, int uid, Entity mesh )
{
	ImGui::PushID( uid );
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex( 0 );
	ImGui::AlignTextToFramePadding();

	auto nameComp = mesh.GetComponent<NameComponent>();
	bool node_open = ImGui::TreeNode(nameComp.Name.c_str());
	/*	ImGui::TableSetColumnIndex( 1 );*/


	if ( ImGui::IsItemClicked() )
	{
		SelectModel( mesh );
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
				TransformComponent& transformComp = m_SelectedModel.GetComponent<TransformComponent>();
				transformComp.Reset();
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

			DeleteMesh( mesh );
			DeselectAll();

	}
}

void Game::DrawModelDetails(Entity Mesh)
{

	JohnMesh* mesh = Mesh.GetComponent<MeshComponent>().Mesh.get();
	John::EAssetType assetType = mesh->GetAssetType ();

	TransformComponent& trans = Mesh.GetComponent<TransformComponent>();

	


	Vector3 pos = trans.GetTranslation();
	Vector3 euler = John::EulerRadiansToDegrees(trans.GetRotationEuler());
	Vector3 scale = trans.GetScale();

	if(ImGui::DragFloat3 ( "Position", &pos.x, .1f ))
	{
		trans.SetTranslation( pos );
	}
	if(ImGui::DragFloat3 ( "Rotation", &euler.x, .1f ))
	{
		trans.SetRotation( John::EulerDegreesToRadians( euler ) );
	}
	if(ImGui::DragFloat3 ( "Scale", &scale.x, .1f ))
	{
		trans.SetScale( scale );
	}



	if(assetType == John::EAssetType::JohnPrimitive)
	{
		JohnPrimitive* prim = static_cast<JohnPrimitive*>(mesh);
		auto device = DX::DeviceResources::Get().GetD3DDevice ();
		float primSize = float(prim->GetSize ());
		int tess = prim->GetTessellation ();
		if(ImGui::DragFloat("Primitive Size", &primSize, .001f))
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
	auto light = Mesh.GetComponent<PointLightComponent>();

	float lightInt = light.LightIntensity;
	Vector3 lightColor = light.LightColor;



}

void Game::DeleteMesh( Entity MeshToDelete )
{
	m_Scene->DestroyEntity ( MeshToDelete );
}



void Game::ConvertMovementDeltaToDragRot(Vector3& InOutDragDelta,  Vector3& OutDrag, Vector3& Rot  )
{
	if(m_MouseState.rightButton && !m_MouseState.leftButton)
	{
		Rot.y = InOutDragDelta.x * m_Camera->m_MovementSettings.MouseLookSensitivity;
		Rot.x = InOutDragDelta.y * m_Camera->m_MovementSettings.MouseLookSensitivity;

	}
}

void Game::SelectModel( Entity ModelToSelect )
{
	if(ModelToSelect.GetEntityHandle () != entt::null)
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

	int x, y;

	//m_MouseDeltaTracker.GetMouseDelta( x, y );
	m_MouseDelta.x = (float)m_MouseState.x;
	m_MouseDelta.y = (float)m_MouseState.y;


}

void Game::DeselectAll()
{
	m_SelectedModel = NullEntity;
}

bool Game::IsCameraViewportHovered() const
{
	ImGuiContext& context = *ImGui::GetCurrentContext ();
	ImGuiWindow* window = context.HoveredWindow;
	if(window)
	{
	return *window->Name == *m_ViewportWindowName.c_str ();

	}
	return false;
}

Entity Game::MousePicking()
{

	auto context = DX::DeviceResources::Get().GetD3DDeviceContext ();

	ImVec2 mousePos = ImGui::GetMousePos ();
	uint32_t x, y;
	x = mousePos.x;
	y = mousePos.y;
	
	float width = m_ViewportSize.x;
	float height = m_ViewportSize.y;

	Matrix proj = m_Camera->GetProjectionMatrix();
	Matrix view = m_Camera->GetViewMatrix();
	Matrix inverseView = view.Invert();

	Vector2 mousePosViewSpace = GetMouseViewportSpace ();
	float pointX = mousePosViewSpace.x;
	float pointY = mousePosViewSpace.y;

	pointX = pointX / proj._11;
	pointY = pointY / proj._22;


	Vector3 rayOrigViewSpace(0.f, 0.f, 0.f);
	Vector3 rayDirViewSpace(pointX, pointY, 1.f);

	Vector3 rayOrigWorldSpace = Vector3::Transform(rayOrigViewSpace, inverseView);
	Vector3 rayDirWorldSpace = Vector3::TransformNormal(rayDirViewSpace, inverseView);

	LineStart = rayOrigWorldSpace;
	LineEnd = (rayDirWorldSpace * 1000.f) + rayOrigWorldSpace;

	rayDirWorldSpace.Normalize();



	float hitDistance = FLT_MAX;

	bool bResult = false;
	Entity ActorToSelect = NullEntity;

	auto ents = m_Scene->GetAllEntitiesWith<MeshComponent>();
	for ( auto ent : ents )
	{
		Entity entity = { ent, m_Scene.get() };
		auto mesh = entity.GetComponent<MeshComponent>().Mesh.get();
		auto trans = entity.GetComponent<TransformComponent> ();
		Matrix world = trans.GetTransformationMatrix();
		Matrix invWorld = world.Invert();


		Vector3 rayOrigLocalSpace = Vector3::Transform(rayOrigWorldSpace, invWorld);
		Vector3 rayDirLocalSpace = Vector3::TransformNormal (rayDirWorldSpace, invWorld);
		rayDirLocalSpace.Normalize ();
		Ray ray;
		ray.direction = rayDirLocalSpace;
		ray.position = rayOrigLocalSpace;
		auto vertices = *mesh->GetVertices();
		auto faces = *mesh->GetFaces();


		for(auto face : faces)
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
				ActorToSelect = entity;
				break;
				
			}
			
			


		}
		if(bResult)
		{
			break;
		}

	}
	if(bResult)
	{
		return ActorToSelect;
	}
	else
	{
		DeselectAll ();
		return NullEntity;
	}
}

WCHAR* Game::ImportFile(COMDLG_FILTERSPEC FileExtension[], UINT ExtensionCount)
{
	IFileOpenDialog* FileOpen;
	HRESULT hr = CoCreateInstance (CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&FileOpen));

	//setup filters
	if(FAILED(hr))
	{
		return nullptr;
	}
	FileOpen->SetFileTypes (ExtensionCount, FileExtension);

	std::string startingDir("Content/");

	PIDLIST_ABSOLUTE pid1;
	WCHAR wstartingDir[MAX_PATH];
	HRESULT parse = SHParseDisplayName (wstartingDir, 0, &pid1, SFGAO_FOLDER, 0);

	if(SUCCEEDED(parse))
	{
		IShellItem* psi;
		parse = SHCreateShellItem (NULL, NULL, pid1, &psi);
		if(SUCCEEDED(parse))
		{
			FileOpen->SetFolder(psi);
		}
		ILFree(pid1);
	}

	if(SUCCEEDED(hr))
	{
		hr = FileOpen->Show(NULL);
		if(SUCCEEDED(hr))
		{
			IShellItem* Item;
			hr = FileOpen->GetResult(&Item);

			if(SUCCEEDED(hr))
			{
				PWSTR filePath;
				char buffer[MAX_PATH];
				hr = Item->GetDisplayName (SIGDN_FILESYSPATH, &filePath);
				if(wcslen(filePath) > 0)
				{
					return filePath;
				}
			}
		}
	}

	return nullptr;
}

void Game::ImportMesh()
{
	auto device = DX::DeviceResources::Get().GetD3DDevice ();
	COMDLG_FILTERSPEC ModelTypes[] =
	{
		{
			L"3D Models", L"*.fbx;*.obj;*.gltf"
		}

	};
	
	WCHAR* ModelName = ImportFile(ModelTypes, _countof(ModelTypes));
	if(ModelName != nullptr)
	{

	std::string name = John::ConvertToUTF8 (std::wstring(ModelName));


	ImportMeshFromFile (name.c_str ());
	}


}

Vector2 Game::GetMouseViewportSpace()
{
	auto [mx, my] = ImGui::GetMousePos();
	const auto& viewportBounds = m_ViewportBounds;
	mx -= viewportBounds[0].x;
	my -= viewportBounds[0].y;
	auto viewportWidth = viewportBounds[1].x - viewportBounds[0].x;
	auto viewportHeight = viewportBounds[1].y - viewportBounds[0].y;

	return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
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
