#include "pch.h"
#include "GUI.h"
#include "JohnCamera.h"
#include "Actor.h"
#include "Primitives.h"
#include "Serialization/SceneSerializer.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

GUI::~GUI()
{
	ImGui_ImplDX11_Shutdown ();
	ImGui_ImplSDL3_Shutdown ();
	ImGui::DestroyContext ();

}

void GUI::Initialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext ();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.f;
		style.Colors[ImGuiCol_WindowBg].w = 1.f;
	}

	ImGui::StyleColorsDark ();

	ImGui_ImplSDL3_InitForD3D (Application::Get().GetWindow ());
	ImGui_ImplDX11_Init (Application::Get().GetDevice (), Application::Get ().GetContext ());
	m_ViewportName = std::string("Viewport");




}

void GUI::TickUI()
{
	ImGui_ImplDX11_NewFrame ();
	ImGui_ImplSDL3_NewFrame ();
	ImGui::NewFrame ();
	ImGuizmo::BeginFrame();
	ImGuiDockNodeFlags NodeFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpaceOverViewport (0, ImGui::GetMainViewport (), NodeFlags);

	DrawViewport();

	DrawSideBar();


	TickMainMenuBar ();

	if(bShowDemoWindow)
	{
		ImGui::ShowDemoWindow (&bShowDemoWindow);
	}
}

void GUI::TickMainMenuBar()
{
	if(ImGui::BeginMainMenuBar ())
	{
		if(ImGui::BeginMenu ("File"))
		{
			if(ImGui::MenuItem("New Scene"))
			{
				Application::Get().NewScene ();
			}
			if(ImGui::MenuItem("Open Scene"))
			{
				COMDLG_FILTERSPEC Extensions[] =
				{
					{

						L"JohnScene", L"*.johnscene"
					}
				};
				wchar_t* SceneName = OpenScene (Extensions, _countof(Extensions));

				if(SceneName != nullptr)
				{
					Application::Get().OpenScene (SceneName);

				}

			}
			if(ImGui::MenuItem("Import Scene"))
			{
				COMDLG_FILTERSPEC Extensions[] =
				{
					{
						L"GLTF", L"*.glb"
					},
					{
						L"FBX", L"*.fbx"
					},
					{
						L"OBJ", L"*.obj"
					}
				};

				wchar_t* SceneName = OpenFile  (Extensions, _countof(Extensions));

				if(SceneName != nullptr)
				{
					Application::Get().ImportScene(SceneName);
				}
			}
			if(ImGui::MenuItem ("Save"))
			{
				SceneSerializer Serializer(m_Scene);

				Serializer.WriteToDisk ("test.johnscene");
			}
			if(ImGui::MenuItem("Quit"))
			{
				Application::Get().SignalQuit ();
			}
			ImGui::EndMenu ();
		}
		if(ImGui::BeginMenu("Help"))
		{
			if(ImGui::MenuItem ("Show ImGuiDemoWindow"))
			{
				bShowDemoWindow = !bShowDemoWindow;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar ();
	}
}

void GUI::TickGizmo()
{
	if(!bIsGizmoActive)
	{
		return;
	}
	ImGuizmo::Enable(true);
	ImGuizmo::SetOrthographic(false);

	ImGuizmo::SetDrawlist();

	float rw = (float)ImGui::GetWindowWidth();
	float rh = (float)ImGui::GetWindowHeight();

	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

	auto camera = Application::Get().GetCamera();

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();

	auto actor = m_SelectedActor;
	if(actor == nullptr)
	{
		return;
	}

	Transform modelTrans = actor->GetTransform();

	Matrix model = modelTrans.ModelMatrix;

	Matrix null = Matrix::Identity;
	bool bIsGizmoHovered = ImGuizmo::IsOver();
	ImGuizmo::SetID(0);
	bool bManipulated = ImGuizmo::Manipulate(*view.m, *proj.m, m_GizmoOperation, m_GizmoMode, *model.m) && !camera->IsInRelativeMode();

	if (bManipulated)
	{
		Vector3 newTrans;
		Quaternion newRot;
		Vector3 newScale;
		model.Decompose(newScale, newRot, newTrans);
		Transform NewTransform;
		actor->SetPosition(newTrans);
		actor->SetRotation(newRot);
		actor->SetScale(newScale);
	}
}

void GUI::SetScene(std::shared_ptr<Scene> InScene)
{
	m_Scene = InScene;
}


void GUI::RenderUI()
{
	auto context = Application::Get().GetContext ();
	
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData (ImGui::GetDrawData ());
	
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows ();
		ImGui::RenderPlatformWindowsDefault ();
	}
}

void GUI::SetViewportTexture(DX::RenderTexture * ViewportImage)
{
	m_ViewportImage = ViewportImage;
}

void GUI::DrawViewport()
{

	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGui::Begin(m_ViewportName.c_str ());

	D3D11_TEXTURE2D_DESC texDesc = {};

	m_ViewportImage->GetRenderTarget()->GetDesc(&texDesc);
	int width = texDesc.Width;
	int height = texDesc.Height;

	size_t regionWidth, regionHeight;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	regionWidth = viewportPanelSize.x;
	regionHeight = viewportPanelSize.y;

	auto windowSize = ImGui::GetWindowSize();
	ImVec2 minBound = ImGui::GetWindowPos();
	ImVec2 viewportOffset = ImGui::GetCursorPos();
	minBound.x -= viewportOffset.x;
	minBound.y -= viewportOffset.y;

	ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };





	if (width != regionWidth || height != regionHeight)
	{
		m_Renderer->OnViewportResize(regionWidth, regionHeight);
		Application::Get().OnViewportResize(regionWidth, regionHeight);
	}

	ImGui::Image((void*)m_ViewportImage->GetShaderResourceView(), ImVec2(regionWidth, regionHeight));

	auto rectMin = ImGui::GetItemRectMin ();
	auto rectMax = ImGui::GetItemRectMax ();
	m_ViewportBounds[0] = { rectMin.x, rectMin.y };
	m_ViewportBounds[1] = { rectMax.x, rectMax.y };
	m_ViewportSize = ImGui::GetCurrentWindow  ()->Size;
	m_ViewportPos = ImGui::GetWindowPos  ();
	TickGizmo();
	ImGui::PopStyleVar ();

	ImGui::End();

}

void GUI::DrawSideBar()
{
	ImGui::Begin("Sidebar");
	
	if(ImGui::Button("Add Plane"))
	{
		std::shared_ptr<Actor> newActor = std::make_shared<Actor>();

		newActor->SetName  ("Plane");
		auto mesh = John::Primities::CreatePlane  (Vector3(3, 3, 3));
		auto renderer = Application::Get().GetRenderer  ();
		mesh->SetMaterial  (renderer->GetDefaultMaterial());
		newActor->SetMesh  (mesh);
		m_Scene->AddActor  (newActor);
	}
	if(ImGui::Button("Add Sphere"))
	{
		std::shared_ptr<Actor> newActor = std::make_shared<Actor> ();
		newActor->SetName  ("Sphere");
		auto mesh = John::Primities::CreateSphere();
		auto renderer = Application::Get().GetRenderer  ();
		mesh->SetMaterial  (renderer->GetDefaultMaterial  ());
		newActor->SetMesh  (mesh);
		m_Scene->AddActor  (newActor);
	}
	ImGuiContext& context = *ImGui::GetCurrentContext  ();
	ImGuiWindow* window = context.HoveredWindow;
	if(window)
	{
		ImGui::Text(window->Name);
	}



	float  x, y;
	SDL_GetMouseState  (&x, &y);

	ImGui::Text("Mouse X: %f, Mouse Y: %f", x, y);

	ImGui::Text("Min X: %f    Min Y: %f", m_ViewportBounds[0].x, m_ViewportBounds[0].y);
	ImGui::Text("Max X: %f    Max Y: %f", m_ViewportBounds[1].x, m_ViewportBounds[1].y);
	ImGui::Text("Pos X: %f    Pos Y: %f", m_ViewportPos.x, m_ViewportPos.y);
	ImGui::Text("Size X: %f    Size Y: %f", m_ViewportSize.x, m_ViewportSize.y);


	ImGui::End();
}


void GUI::SetRenderer(std::shared_ptr<SceneRenderer> renderer)
{
	m_Renderer = renderer;
}

void GUI::GetViewportDimensions(int & width, int & height)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	m_ViewportImage->GetRenderTarget()->GetDesc(&texDesc);
	width = texDesc.Width;
	height = texDesc.Height;
}

bool GUI::IsViewportHovered() const
{
	ImGuiContext& context = *ImGui::GetCurrentContext ();
	ImGuiWindow* window = context.HoveredWindow;
	if(window)
	{
		bool ret = *window->Name == *m_ViewportName.c_str ();
		return ret;
	}
	return false;
}

void GUI::MousePicking()
{
	auto context = Application::Get().GetContext();
	if (IsViewportHovered() && !ImGuizmo::IsUsingAny())
	{
		ImVec2 mousePos = ImGui::GetMousePos();



		uint32_t MouseX, MouseY;
		MouseX= 0.f;
		MouseY = 0.f;
		
		int width, height;
		GetViewportDimensions(width, height);

		auto camera = Application::Get().GetCamera();

		Matrix view = camera->GetViewMatrix();
		Matrix proj = camera->GetProjectionMatrix();
		Matrix inverseView= view.Invert ();
		Matrix inverseProj = proj.Invert ();

		Vector2 mousePosViewSpace= GetMousePosViewportSpace();

		float pointX = mousePosViewSpace.x;
		float pointY =  mousePosViewSpace.y;
		pointX = pointX / proj._11;
		pointY = pointY / proj._22;

		Vector3 rayOriginViewSpace(0.f, 0.f, 0.f);
		Vector3 rayDirViewSpace(pointX, pointY, 1.f);

		Vector3 rayOriginWorldSpace = Vector3::Transform(rayOriginViewSpace, inverseView);
		Vector3 rayDirWorldSpace = Vector3::TransformNormal(rayDirViewSpace, inverseView);

		rayDirWorldSpace.Normalize();

		float hitDistance = FLT_MAX;

		bool bResult = false;

		std::shared_ptr<Actor> actortoSelect = nullptr;

		for (auto actor : m_Scene->m_Actors)
		{
			Transform trans = actor->GetTransform();
			std::shared_ptr<JohnMesh> mesh = actor->GetMesh();
			if (!mesh)
			{
				continue;
			}
			Matrix world = trans.ModelMatrix;
			Matrix inverseWorld = world.Invert();





			Vector3 rayOriginLocalSpace = Vector3::Transform(rayOriginWorldSpace, inverseWorld);
			Vector3 rayDirLocalSpace = Vector3::TransformNormal(rayDirWorldSpace, inverseWorld);

			rayDirLocalSpace.Normalize();

			Ray ray;
			ray.direction = rayDirLocalSpace;
			ray.position = rayOriginLocalSpace;

			auto vertices = mesh->m_Vertices;
			auto faces = mesh->m_Indices;

			for (int i = 0; i < faces.size() - 1; i += 3)
			{
				unsigned int Index1 = faces[i];
				unsigned int Index2 = faces[i+1];
				unsigned int Index3 = faces[i+2];

				Vertex vert1 = vertices[Index1];
				Vertex vert2 = vertices[Index2];
				Vertex vert3 = vertices[Index3];



				float dist = 100.f;
				bResult = ray.Intersects(vert1.Position, vert2.Position, vert3.Position, dist);
				bool bHitTriangle = false;
				if (bResult)
				{
					hitDistance = std::min(hitDistance, dist);
					SelectActor(actor);
					bHitTriangle = true;
					break;
				}
				XMVECTORF32 Color;
				if (bHitTriangle)
				{
					Color = DirectX::Colors::Red;
				}
				else
				{
					Color = DirectX::Colors::White;
				}
			}
			if(bResult)
			{
				bClickedOnObject = true;
				break;
			}
			else
			{
				bClickedOnObject = false;
				DeselectAll  ();
			}

		}


	}
}

DirectX::SimpleMath::Vector2 GUI::GetMousePosViewportSpace() const
{
	ImVec2 MousePos = ImGui::GetMousePos ();

	const auto& viewportBounds = m_ViewportBounds;
	MousePos.x -= viewportBounds[0].x;
	MousePos.y -= viewportBounds[0].y;

	auto viewportWidth = viewportBounds[1].x - viewportBounds[0].x;
	auto viewportHeight = viewportBounds[1].y - viewportBounds[0].y;

	Vector2 MousePosViewSpace;
	MousePosViewSpace.x = (MousePos.x / viewportWidth) * 2.f - 1.f;
	MousePosViewSpace.y = ((MousePos.y / viewportHeight) * 2.f - 1.f) * -1.f;
	return MousePosViewSpace;
}

bool GUI::HasActorSelected() const
{
	if(m_SelectedActor != nullptr)
	{
		return true;
	}
	return false;
}

std::shared_ptr<Actor> GUI::GetSelectedActor()
{
	return m_SelectedActor;

}




void GUI::SwitchGizmoOperation(SDL_Keycode key)
{
	if(key == SDLK_q)
	{
		bIsGizmoActive = false;
	}
	else
	{
		bIsGizmoActive = true;
	}
	if(key == SDLK_w)
	{
		m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	}
	if(key == SDLK_e)
	{
		m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
	}
	if(key == SDLK_r)
	{
		m_GizmoOperation = ImGuizmo::OPERATION::SCALE;
	}
}

wchar_t* GUI::OpenScene(COMDLG_FILTERSPEC FileExt[], UINT ExtensionCount)
{
	IFileOpenDialog* FileOpen;
	HRESULT hr = CoCreateInstance (CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&FileOpen));

	//setup filters
	if (FAILED(hr))
	{
		return nullptr;
	}
	FileOpen->SetFileTypes (ExtensionCount, FileExt);

	std::string startingDir("Content/");

	PIDLIST_ABSOLUTE pid1;
	WCHAR wstartingDir[MAX_PATH];
	HRESULT parse = SHParseDisplayName (wstartingDir, 0, &pid1, SFGAO_FOLDER, 0);

	if(FAILED(hr))
	{
		std::printf("Could not parse display name");
		return nullptr;
	}

	IShellItem* psi;
	hr = SHCreateShellItem (NULL, NULL, pid1, &psi);
	if(SUCCEEDED(hr))
	{
		FileOpen->SetFolder (psi);
	}
	ILFree(pid1);

	hr = FileOpen->Show(NULL);
	if(FAILED(hr))
	{
		return nullptr;
	}
	IShellItem* Item;
	hr = FileOpen->GetResult (&Item);

	if(FAILED(hr))
	{
		std::printf("Could not get file dialog item");
		return nullptr;
	}
	PWSTR filePath;
	char buffer[MAX_PATH];
	hr = Item->GetDisplayName (SIGDN_FILESYSPATH, &filePath);
	if(wcslen(filePath) > 0)
	{


		Item->Release ();
		FileOpen->Release ();
		return filePath;
	}

	Item->Release ();
	psi->Release ();
	FileOpen->Release ();
	return nullptr;
}

wchar_t* GUI::OpenFile(COMDLG_FILTERSPEC FileExt[], UINT ExtensionCount)
{
	IFileOpenDialog* FileOpen;
	HRESULT hr = CoCreateInstance  (CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&FileOpen));

	if(FAILED(hr))
	{
		std::printf("Could not create instance!");
		return nullptr;
	}

	hr = FileOpen->SetFileTypes(ExtensionCount, FileExt);

	std::string startingDir("Content/");

	PIDLIST_ABSOLUTE pid1;
	WCHAR startingDirW[MAX_PATH];
	HRESULT parse = SHParseDisplayName (startingDirW, 0, &pid1, SFGAO_FOLDER, 0);

	if(SUCCEEDED(parse))
	{
		IShellItem* psi;
		parse = SHCreateShellItem(NULL, NULL, pid1, &psi);
		if(SUCCEEDED(parse))
		{
			FileOpen->SetFolder  (psi);

		}
		ILFree(pid1);
	}





	hr = FileOpen->Show(NULL);
	if(FAILED(hr))
	{
		std::printf("Could not show file!");
		return nullptr;
	}
	IShellItem* Item;

	hr = FileOpen->GetResult  (&Item);

	if(FAILED(hr))
	{
		std::printf("Could not get item!");
		return nullptr;
	}

	PWSTR filePath;
	char buffer[MAX_PATH];
	hr = Item->GetDisplayName  (SIGDN_FILESYSPATH, &filePath);

	if(wcslen(filePath) > 0)
	{
		Item->Release();
		FileOpen->Release();
		return filePath;
	}

	Item->Release  ();
	FileOpen->Release  ();
	return nullptr;


}

void GUI::SelectActor(std::shared_ptr<Actor> actor)
{
	m_SelectedActor = actor;

}

void GUI::ClearScene()
{
	m_SelectedActor.reset();
	m_Scene.reset();
}

void GUI::DeselectAll()
{
	m_SelectedActor = nullptr;
}

void GUI::GetViewportBounds(ImVec2& min, ImVec2& max)
{
	min = m_ViewportBounds[0];
	max = m_ViewportBounds[1];
}

ImVec2 GUI::GetViewportSize() const
{
	return m_ViewportSize;
}

ImVec2 GUI::GetViewportPos() const
{
	return m_ViewportPos;

}

bool GUI::IsGizmoBeginUsed()
{
	return ImGuizmo::IsOver();

}
