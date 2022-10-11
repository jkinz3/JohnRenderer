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
	m_Camera->SetRotation( Vector2( 0.f, 180.f ) );

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


	
	ImGui::Begin( "Editor" );
	for(auto mesh : m_Meshes)
	{
		DrawModelDetails(mesh.get());

	}
	ImGui::DragFloat3( "Light Position", &m_LightPos.x, .1f );
	ImGui::End();

}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

	Movement( elapsedTime );

    // TODO: Add your game logic here.
    elapsedTime;
}

void Game::Movement( float DeltaTime )
{
	auto keyboard = m_Keyboard->GetState();
	auto mouse = m_Mouse->GetState();
	m_Keys.Update( keyboard );
	m_MouseButtons.Update( mouse );
	if(m_Keys.pressed.R)
	{
		ReloadShaders();
	}

	m_MouseDelta.x =  (float)mouse.x ;
	m_MouseDelta.y = (float) mouse.y ;
	
	if (mouse.positionMode == Mouse::MODE_RELATIVE)
	{
		if ( keyboard.LeftAlt )
		{
			Vector2 delta = m_MouseDelta * .003f;
			if ( mouse.middleButton )
			{
				m_Camera->MousePan( delta );
			}
			else if ( mouse.leftButton )
			{
				m_Camera->MouseOrbit( delta );
			}
			else if ( mouse.rightButton )
			{
				m_Camera->Mousezoom( delta );
			}

		}
		else
		{


			Vector2 delta = m_MouseDelta * DeltaTime;

			Vector3 move = Vector3::Zero;


			if ( keyboard.E )
			{
				move += (Vector3::Up * DeltaTime);
			}
			if ( keyboard.Q )
			{
				move -= Vector3::Up * DeltaTime;
			}
			if ( keyboard.A )
			{
				move -= m_Camera->GetRightVector() * DeltaTime;
			}
			if ( keyboard.D )
			{
				move += m_Camera->GetRightVector() * DeltaTime;
			}
			if ( keyboard.W )
			{
				move += m_Camera->GetForwardVector() * DeltaTime;
			}
			if ( keyboard.S )
			{
				move -= m_Camera->GetForwardVector() * DeltaTime;
			}

			if ( mouse.scrollWheelValue != 0 )
			{
				int x = 5;
			}
			m_Camera->MoveAndRotateCamera( move, delta );
		}
	}
	m_bIsRelativeMode = mouse.rightButton || keyboard.LeftAlt && (mouse.leftButton || mouse.middleButton || mouse.rightButton);

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

	context->UpdateSubresource( m_PhongShadingCB.Get(), 0, nullptr, &shadingConstants, 0, 0 );
	for(auto mesh : m_Meshes)
	{
		DrawMesh( mesh.get() );
	}
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

	context->UpdateSubresource( m_PhongTransformCB.Get(), 0, nullptr, &transformConstants, 0, 0 );


	context->VSSetConstantBuffers( 0, 1, m_PhongTransformCB.GetAddressOf() );
	context->PSSetConstantBuffers( 0, 1, m_PhongShadingCB.GetAddressOf() );
	context->VSSetShader( m_PhongProgram.VertexShader.Get(), nullptr, 0 );
	context->VSSetShader( m_PhongProgram.VertexShader.Get(), nullptr, 0 );
	context->PSSetShader( m_PhongProgram.PixelShader.Get(), nullptr, 0 );
	context->IASetInputLayout( m_PhongProgram.InputLayout.Get() );
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
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;

	std::shared_ptr<JohnMesh> MonkeyMesh = John::LoadMeshFromFile( "Content/monkey.obj" );
	MonkeyMesh->Build( device );

	m_Meshes.push_back( MonkeyMesh );

	m_PhongProgram = John::CreateShaderProgram( device, L"Shaders/PhongVS.hlsl", L"Shaders/PhongPS.hlsl" );

	m_PhongTransformCB = John::CreateConstantBuffer<John::PhongTransformCB>( device );
	m_PhongShadingCB = John::CreateConstantBuffer<John::PhongShadingCB>( device );



	
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.

	auto rect = m_deviceResources->GetOutputSize();

	if(m_Camera)
	{
		m_Camera->SetImageSize( rect.right, rect.bottom );
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
