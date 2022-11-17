//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Resources.h"
#include "JohnMesh.h"

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
	m_Camera->SetRotation( Vector2( 0.f, XM_PI) );

	m_LightPos = Vector3( .6f, 1.f, 2.f );

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
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
	if(rdoc_api)
	{
		rdoc_api->StartFrameCapture( NULL, NULL );
	}
#endif // DEBUG

	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_Environment = John::CreateEnvironmentFromFile( device, context, EnvMapFile );
	m_brdfSampler = John::CreateSamplerState( device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP );
	m_Sky = GeometricPrimitive::CreateGeoSphere( context, 2.f, 3.f, true );
	m_SkyEffect = std::make_unique<DX::SkyboxEffect>( device );

	m_Sky->CreateInputLayout( m_SkyEffect.get(), m_SkyInputLayout.ReleaseAndGetAddressOf() );

	m_SkyEffect->SetTexture( m_Environment.SpecularIBL.SRV.Get() );


#ifdef _DEBUG
	if(rdoc_api)
	{
		rdoc_api->EndFrameCapture( NULL, NULL );
	}
#endif // DEBUG


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
		ImGui::TreePop();

	}



	
	ImGui::Text( "Camera Rotation: %f, %f", m_Camera->GetRotation().x, m_Camera->GetRotation().y );

	if ( ImGui::BeginMenu( "Switch Shader" ) )
	{
		ImGui::RadioButton( "Blinn Phong", &m_CurrentShaderIndex, 0 );
		ImGui::RadioButton( "PBR", &m_CurrentShaderIndex, 1 );

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
	
	if(m_MouseButtons.leftButton == Mouse::ButtonStateTracker::PRESSED)
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
	
	if(m_Keys.pressed.R)
	{
		ReloadShaders();
	}

	int x, y;
	m_MouseDeltaTracker.GetMouseDelta( x, y );
	m_MouseDelta.x = (float)x;
	m_MouseDelta.y = (float)y;

	
	if(m_MouseButtons.rightButton == Mouse::ButtonStateTracker::PRESSED)
	{
		m_DragAmount = Vector2::Zero;
	}

	if(m_MouseState.positionMode == Mouse::MODE_RELATIVE)
	{
		m_DragAmount += m_MouseDelta;
	}

	if (m_MouseState.positionMode == Mouse::MODE_RELATIVE)
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
		else
		{

			Vector2 delta = m_MouseDelta;

			Vector3 move = Vector3::Zero;


			if ( m_KeyboardState.E )
			{
				move += (Vector3::Up * DeltaTime);
			}
			if ( m_KeyboardState.Q )
			{
				move -= Vector3::Up * DeltaTime;
			}
			if ( m_KeyboardState.A )
			{
				move -= m_Camera->GetRightVector() * DeltaTime;
			}
			if ( m_KeyboardState.D )
			{
				move += m_Camera->GetRightVector() * DeltaTime;
			}
			if ( m_KeyboardState.W )
			{
				move += m_Camera->GetForwardVector() * DeltaTime;
			}
			if ( m_KeyboardState.S )
			{
				move -= m_Camera->GetForwardVector() * DeltaTime;
			}
			m_Camera->MoveAndRotateCamera( move, delta );
			if ( m_MouseDelta != Vector2::Zero )
			{
				m_bWasCameraMoved = true;
			}
			else
			{
				m_bWasCameraMoved = false;
			}



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

	if(m_Keys.pressed.F)
	{
		m_Camera->FocusOnPosition( Vector3::Zero );
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

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    context;

	DrawScene();
	
	DrawSky();


    m_deviceResources->PIXEndEvent();

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
}

void Game::DrawSky()
{
	m_SkyEffect->SetView( m_Camera->GetViewMatrix());
	m_SkyEffect->SetProjection( m_Camera->GetProjectionMatrix() );
	m_Sky->Draw( m_SkyEffect.get(), m_SkyInputLayout.Get() );
}

void Game::DrawMesh( JohnMesh* MeshToDraw )
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

	ID3D11ShaderResourceView* const srvs[] =
	{
		m_BrickNormal.Get()
	};

	ID3D11SamplerState* const states[] =
	{
		m_StandardSampler.Get()
	};

	John::ShaderProgram ProgramToUse = m_Shaders.find(m_CurrentShaderIndex)->second;

	context->PSSetShaderResources(0, _countof(srvs), srvs);
	context->PSSetSamplers( 0, _countof( states ), states );
	context->VSSetConstantBuffers( 0, 1, m_TransformCB.GetAddressOf() );
	context->PSSetConstantBuffers( 0, 1, m_ShadingCB.GetAddressOf() );
	context->VSSetShader( ProgramToUse.VertexShader.Get(), nullptr, 0 );
	context->VSSetShader( ProgramToUse.VertexShader.Get(), nullptr, 0 );
	context->PSSetShader( ProgramToUse.PixelShader.Get(), nullptr, 0 );
	context->IASetInputLayout( ProgramToUse.InputLayout.Get() );


	MeshToDraw->Draw( context );

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

void Game::OnMouseMove()
{
	m_MouseDeltaTracker.UpdateState( m_Mouse->GetState());
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

    // TODO: Initialize device dependent objects here (independent of window size).
    device;

	std::shared_ptr<JohnMesh> MonkeyMesh = John::LoadMeshFromFile( "Content/sphere.obj" );
	MonkeyMesh->Build( device );

	m_Meshes.push_back( MonkeyMesh );

	m_PhongProgram = John::CreateShaderProgram( device, L"Shaders/PhongVS.hlsl", L"Shaders/PhongPS.hlsl" );

	m_PBRProgram = John::CreateShaderProgram( device, L"Shaders/PBRVS.hlsl", L"Shaders/PBRPS.hlsl" );

	m_Shaders.emplace(0, m_PhongProgram);
	m_Shaders.emplace(1, m_PBRProgram);

	m_TransformCB = John::CreateConstantBuffer<John::PhongTransformCB>( device );
	m_ShadingCB = John::CreateConstantBuffer<John::PhongShadingCB>( device );

	DX::ThrowIfFailed(
		CreateWICTextureFromFile(device, L"Content/Brick_Wall_Normal.jpg", nullptr, m_BrickNormal.ReleaseAndGetAddressOf())
	);

	InitializeSky( "Content/environment.hdr" );

	m_StandardSampler = John::CreateSamplerState(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
	
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
}


bool Game::CanMoveCamera() const
{
	return m_bIsRelativeMode;
}

void Game::ReloadShaders()
{
	m_PhongProgram.VertexShader.Reset();
	m_PhongProgram.PixelShader.Reset();
	m_PhongProgram.InputLayout.Reset();
	
	m_PhongProgram = John::CreateShaderProgram(m_deviceResources->GetD3DDevice(), m_PhongProgram.VertFileName, m_PhongProgram.PixelFileName );


		m_PBRProgram.VertexShader.Reset();
		m_PBRProgram.PixelShader.Reset();
		m_PBRProgram.InputLayout.Reset();

		m_PBRProgram = John::CreateShaderProgram( m_deviceResources->GetD3DDevice(), m_PBRProgram.VertFileName, m_PBRProgram.PixelFileName );
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
	bool bManipulated = ImGuizmo::Manipulate( *view.m, *proj.m, m_CurrentGizmoOperation, m_CurrentGizmoMode, *model.m );

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

void Game::DrawModelInOutliner( const char* prefix, int uid, std::shared_ptr<JohnMesh> mesh )
{
	ImGui::PushID( uid );
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex( 0 );
	ImGui::AlignTextToFramePadding();
	bool node_open = ImGui::TreeNode( mesh->GetName().c_str() );
/*	ImGui::TableSetColumnIndex( 1 );*/
	
	if(node_open)
	{
		if ( ImGui::IsItemClicked() )
		{
			SelectModel( mesh.get() );
		}

		bool bEntityDeleted = false;
		if(ImGui::BeginPopupContextItem())
		{
			if(ImGui::MenuItem("DeleteActor"))
			{
				bEntityDeleted = true;
			}
			if(ImGui::MenuItem("Reset Transform"))
			{
				if(m_SelectedModel)
				{
					m_SelectedModel->ResetTransformations();
				}

			}
			if ( bEntityDeleted )
			{
				if ( m_SelectedModel )
				{
					DeleteMesh( mesh );
					DeselectAll();
				}
			}
			ImGui::EndPopup();
		}


		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Game::DrawModelDetails(JohnMesh* Mesh)
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
}

void Game::DeleteMesh( std::shared_ptr<JohnMesh> MeshToDelete )
{
	m_Meshes.erase( std::remove( m_Meshes.begin(), m_Meshes.end(), MeshToDelete ) );
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

JohnMesh* Game::MousePicking()
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
	JohnMesh* ActorToSelect = nullptr;

	for(auto& mesh : m_Meshes)
	{
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
				ActorToSelect = mesh.get();
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
