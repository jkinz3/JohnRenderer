#pragma once


#include <stdio.h>
#include <filesystem>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <ShObjIdl.h>
#include <ShlObj_core.h>

#include "BufferHelpers.h"
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"
#include "ReadData.h"
#include "SkyboxEffect.h"
#include "Application.h"
#include "DebugDraw.h"
#include "RenderTexture.h"
#include <sstream>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl2.h"
#include "ImGuizmo.h"

class com_exception : public std::exception
{
public:
	com_exception(HRESULT hr) noexcept :result(hr)
	{

	}
	const char* what() const noexcept override
	{
		static char s_str[64] = {};
		sprintf_s(s_str, "Failure with an HRESULT of %08X", static_cast<unsigned int>(result));
		return s_str;
	}
private:

	HRESULT result;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if(FAILED(hr))
	{
		throw com_exception(hr);
	}
}


#include <string.h>