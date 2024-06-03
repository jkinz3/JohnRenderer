#include "pch.h"
#include "Application.h"
#include "JohnCamera.h"
#include "GUI.h"
#include "Serialization\SceneSerializer.h"

namespace
{
#ifdef _DEBUG
	inline bool AreSDKLayersAvailable() noexcept
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,
			nullptr,
			D3D11_CREATE_DEVICE_DEBUG,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			nullptr,
			nullptr,
			nullptr
		);
		return SUCCEEDED(hr);
	}
#endif
}

Application::Application(int width, int height, const char* name)
	:m_Width(width), m_Height(height), m_AppName(name)
{

}

bool Application::Initialize()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		printf("Error: %s\n", SDL_GetError ());
		return false;
	}

	SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	m_Window = SDL_CreateWindow(m_AppName.c_str (), m_Width, m_Height, flags);
	if(!m_Window)
	{
		printf("Error: %s\n", SDL_GetError ());
		return false;
	}

	auto hWnd = (HWND)SDL_GetProperty(SDL_GetWindowProperties(m_Window), "SDL.window.win32.hwnd", NULL);


	if(!InitD3D (hWnd))
	{
		CleanupD3D ();
		return false;
	}

	m_Scene = std::make_shared<Scene>();
	m_Scene->LoadFromFile ("Content/scene.glb");
	m_Renderer = std::make_shared<SceneRenderer>();
	m_Renderer->Initialize();


	m_GUI = std::make_shared<GUI>();
	m_GUI->Initialize ();
	m_GUI->SetScene (m_Scene);
	m_GUI->SetViewportTexture(m_Renderer->GetViewportRenderTarget());
	m_GUI->SetRenderer(m_Renderer);

	m_Camera = std::make_shared<JohnCamera>(Vector3(0.f, 0.f, 3.f), Vector3(0.f, 180.f, 0.f));
	return true;


}

bool Application::InitD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = m_Width;
	sd.BufferDesc.Height = m_Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	sd.BufferDesc.RefreshRate.Numerator =  60;
	sd.BufferDesc.RefreshRate.Denominator =  1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT createFlags = 0;
#ifdef _DEBUG
	if(AreSDKLayersAvailable())
	{
		createFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	else
	{
		OutputDebugStringA ("WARNING: Direct3D Debug Device aint available!\n");
	}
#endif

	D3D_FEATURE_LEVEL featLevel;
	const D3D_FEATURE_LEVEL featLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	HRESULT hr = D3D11CreateDeviceAndSwapChain (NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createFlags, featLevels, 2, D3D11_SDK_VERSION,
		&sd, m_SwapChain.ReleaseAndGetAddressOf (), m_Device.ReleaseAndGetAddressOf (), &featLevel, m_Context.ReleaseAndGetAddressOf ());

	if(hr != S_OK)
	{
		return false;
	}

#ifdef _DEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	if(SUCCEEDED(m_Device.As(&d3dDebug)))
	{
		ComPtr<ID3D11InfoQueue> infoQueue;
		if(SUCCEEDED(d3dDebug.As(&infoQueue)))
		{
			infoQueue->SetBreakOnSeverity (D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity (D3D11_MESSAGE_SEVERITY_ERROR, true);
		}
		D3D11_MESSAGE_ID hide[] =
		{
			D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
		};
		D3D11_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		infoQueue->AddStorageFilterEntries (&filter);
	}
#endif

	ThrowIfFailed (m_Context.As(&m_DebugAnnotation));


	CreateRenderTarget ();


	return true;
}


void Application::CleanupD3D()
{
	if(m_Context)
	{
		m_Context->ClearState ();
	}
	m_SwapChain.Reset();
	m_BackBuffer.Reset();
	m_Context.Reset ();
	m_Device.Reset ();

	
	return;

}

void Application::Cleanup()
{
	SDL_DestroyWindow (m_Window);
	SDL_Quit ();
	CleanupD3D ();

}

void Application::CleanupRenderTarget()
{
	m_BackBuffer.Reset();
}
void Application::CreateRenderTarget()
{
	HWND hWnd = GetNativeWindow  ();
	RECT rc;
	GetClientRect (hWnd, &rc);

	m_Width = rc.right;
	m_Height = rc.bottom;

	

	ID3D11Texture2D* backbuffer;
	HRESULT hr = m_SwapChain->GetBuffer (0, IID_PPV_ARGS(&backbuffer));
	if(hr != S_OK)
	{
		return;
	}
	hr = m_Device->CreateRenderTargetView (backbuffer, NULL, m_BackBuffer.ReleaseAndGetAddressOf ());
	if(hr != S_OK)
	{
		return;
	}
	backbuffer->Release ();




}

void Application::OnWindowResized(int width, int height)
{
	CleanupRenderTarget ();
	m_SwapChain->ResizeBuffers (0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	CreateRenderTarget ();
	m_Camera->UpdateProj();
	
}

void Application::GetViewportDimensions(int & width, int & height)
{
	m_GUI->GetViewportDimensions(width, height);
}

void Application::OnViewportResize(int width, int height)
{
	m_Camera->UpdateProj();
}


void Application::ProcessInput(float DeltaTime)
{
	SDL_Event event;
	Vector2 MouseDelta = Vector2::Zero;
	while(SDL_PollEvent (&event))
	{
		ImGui_ImplSDL3_ProcessEvent (&event);
		if(event.type == SDL_EVENT_QUIT)
		{
			bWantsToQuit = true;
		}

		if(event.type == SDL_EVENT_WINDOW_RESIZED && event.window.windowID == SDL_GetWindowID(m_Window))
		{
			OnWindowResized (event.window.data1, event.window.data2);

		}
		if(event.type == SDL_EVENT_MOUSE_MOTION)
		{
			MouseDelta.x = event.motion.xrel;
			MouseDelta.y = event.motion.yrel;

		}

		if(event.type == SDL_EVENT_KEY_DOWN)
		{
			switch(event.key.keysym.sym)
			{
			case SDLK_F1:
				m_Renderer->SwitchToLit ();
				break;
			case SDLK_F2:
				m_Renderer->SwitchToWireframe ();
				break;
			case SDLK_f:
				if(m_GUI->HasActorSelected ())
				{
					m_Camera->FocusOnPosition (m_GUI->GetSelectedActor ()->GetPosition ());

				}
				break;
			case SDLK_ESCAPE:
				m_GUI->DeselectAll  ();

			case SDLK_PERIOD:
				{

				auto mods = SDL_GetModState ();
				if(mods & SDL_KMOD_LCTRL && mods & SDL_KMOD_LSHIFT)
				{
					m_Renderer->RecompileShaders();

				}
				break;
				}
			case SDLK_q:
			case SDLK_w:
			case SDLK_e:
			case SDLK_r:
				if(!m_Camera->IsInRelativeMode ())
				{
					m_GUI->SwitchGizmoOperation(event.key.keysym.sym);
				}
				break;
			}
		}
		if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			if (event.button.button == SDL_BUTTON_LEFT &&  m_TrackingDelta == Vector2::Zero)
			{
				m_GUI->MousePicking();
			}
		}
		if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			if(SDL_GetRelativeMouseMode() == SDL_FALSE)
			{
				m_TrackingDelta = Vector2::Zero;
			}
		}
	}
	m_Camera->SetCanMoveCamera (m_GUI->IsViewportHovered () || !m_Camera->IsInRelativeMode ());
	m_Camera->Tick(DeltaTime, MouseDelta);

	if(SDL_GetRelativeMouseMode() == SDL_TRUE)
	{
		m_TrackingDelta += MouseDelta;
	}

}

void Application::Clear()
{
	float clearColor[4] = {.2f, .2f, .2f, 1.f};
	auto rt = m_Renderer->GetPrePostProccessTarget ()->GetRenderTargetView ();
	m_Context->ClearRenderTargetView (rt, clearColor);
	auto bb = m_BackBuffer.Get();
	m_Context->ClearRenderTargetView(bb, clearColor);


	m_Renderer->ClearRenderTargets();
}

void Application::Render()
{

	D3D11_VIEWPORT viewport =
	{
		0.f,
		0.f,
		(float)m_Width,
		(float)m_Height,
		0.1, 
		1.f
	};

	m_Context->RSSetViewports (1, &viewport);



	m_Renderer->DrawScene(m_Scene.get(), m_Camera->GetViewMatrix(), m_Camera->GetProjectionMatrix());


	m_Renderer->DrawPostProcess ();


	m_Context->OMSetRenderTargets (1, m_BackBuffer.GetAddressOf(), nullptr);
	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	m_Context->PSSetShaderResources (0, 1, nullsrv);

	m_GUI->SetViewportTexture(m_Renderer->GetViewportRenderTarget());



	m_GUI->RenderUI ();
}

void Application::Present()
{
	m_SwapChain->Present (1, 0);
}

void Application::Run()
{
	uint64_t oldTime = SDL_GetTicks ();

	while(!bWantsToQuit)
	{
		uint64_t newTime = SDL_GetTicks ();
		float deltaTime = newTime - oldTime;
		deltaTime = deltaTime / 1000.f;
		oldTime = newTime;

		Tick(deltaTime);
	}
	Cleanup();
}

void Application::Tick(float DeltaTime)
{
	ProcessInput (DeltaTime);
	if(bWantsToQuit)
	{
		return;
	}
	m_GUI->TickUI ();
	Clear();
	Render();
	Present ();
}

Application& Application::Get()
{
	if(Inst != nullptr)
	{
		return *Inst;
	}
	else
	{
		Inst = new Application(1280, 720, "JohnRenderer");
		return *Inst;
	}
}

void Application::CleanupApplication()
{
	if(Inst)
	{
		delete Inst;
		Inst = nullptr;
	}
}

ID3D11Device* Application::GetDevice()
{
	return m_Device.Get();
}

ID3D11DeviceContext* Application::GetContext()
{
	return m_Context.Get();
}

void Application::GetBackBufferSize(int& width, int& height)
{
	width = m_Width;
	height = m_Height;

}

SDL_Window* Application::GetWindow() const
{
	return m_Window;
}

void Application::SignalQuit()
{
	bWantsToQuit = true;
}

std::shared_ptr<SceneRenderer> Application::GetRenderer() const
{
	return m_Renderer;
}

HWND Application::GetNativeWindow() const
{
	auto hWnd = (HWND)SDL_GetProperty(SDL_GetWindowProperties(m_Window), "SDL.window.win32.hwnd", NULL);


	return hWnd;
}

void Application::RequestExit()
{
	Cleanup ();
	bWantsToQuit = true;
}

void Application::BeginDebugEvent(_In_z_ const wchar_t* name)
{
	m_DebugAnnotation->BeginEvent (name);
}

void Application::EndDebugEvent()
{
	m_DebugAnnotation->EndEvent ();
}

void Application::SetDebugMarker(_In_z_ const wchar_t* name)
{
	m_DebugAnnotation->SetMarker (name);
}

std::shared_ptr<Actor> Application::GetSelectedActor() const
{
	return m_GUI->GetSelectedActor ();

}

bool Application::IsActorSelected() const
{
	return m_GUI->HasActorSelected ();

}

void Application::OpenScene(const std::wstring FileName)
{
	SceneSerializer Serializer;
	std::string file(FileName.begin(), FileName.end());
	m_Scene = Serializer.LoadFromDisk (file);
}

void Application::NewScene()
{
	m_GUI->ClearScene  ();
	long count = m_Scene.use_count  ();
	m_Scene.reset();

	m_Scene = std::make_shared<Scene>();
	m_GUI->SetScene  (m_Scene);
}
