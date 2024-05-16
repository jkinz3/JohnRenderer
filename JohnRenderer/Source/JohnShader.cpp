#include "pch.h"
#include "JohnShader.h"
#include "Texture.h"

JohnShader::JohnShader()
	:m_TransformCB(Application::Get().GetDevice ()), m_ShadingCB(Application::Get().GetDevice ())
{

}

void JohnShader::Apply()
{
	auto context = Application::Get ().GetContext ();
	context->IASetInputLayout (m_InputLayout.Get ());
	context->PSSetShader (m_PixelShader.Get(), nullptr, 0);
	context->VSSetShader (m_VertexShader.Get(), nullptr, 0);

	TransformConstants transConst = {};
	Matrix MVP = m_Model * m_View * m_Proj;
	transConst.MVP = MVP.Transpose ();
	transConst.Model = m_Model.Transpose ();

	m_TransformCB.SetData (context, transConst);

	auto transCB = m_TransformCB.GetBuffer ();
	context->VSSetConstantBuffers (0, 1, &transCB);

	ShadingConstants shadeConst = {};
	Matrix invView = m_View.Invert();
	shadeConst.CamPos = Vector3(invView._41, invView._42, invView._43);
	shadeConst.NumPointLights = m_NumPointLights;

	m_ShadingCB.SetData (context, shadeConst);

	auto shadeCB = m_ShadingCB.GetBuffer ();

	context->PSSetConstantBuffers (0, 1, &shadeCB);

	ID3D11ShaderResourceView* const srvs[] = 
	{
		m_DiffIBL.Get(),
		m_SpecIBL.Get()

	};

	context->PSSetShaderResources (4, _countof(srvs), srvs);

	ID3D11SamplerState* const samplers[] = 
	{
		m_DefaultSampler.Get()
	};

	context->PSSetSamplers (0, _countof(samplers), samplers);
}

void JohnShader::LoadFromFiles(std::wstring VertFile, std::wstring PixelFile)
{
	std::wstring runtimePath = L"Shaders/Runtime/";

	//strip .cso extension and replace with hlsl

	size_t vertlastDot = VertFile.find_last_of (L".");
	size_t pixellastDot = PixelFile.find_last_of (L".");

	std::wstring vertrawFileName = VertFile.substr (0, vertlastDot);
	std::wstring pixelrawFileName = PixelFile.substr (0, pixellastDot);
	std::wstring hlsl(L".hlsl");
	std::wstring sourceDir(L"Shaders/Source/");
	m_VertFileSource =  sourceDir + vertrawFileName + hlsl;
	m_PixelFileSource = sourceDir + pixelrawFileName + hlsl;


	std::wstring vertPath = runtimePath + VertFile;
	std::wstring pixelPath = runtimePath + PixelFile;

	auto device = Application::Get().GetDevice ();

	auto vsBlob = DX::ReadData (vertPath.c_str ());

	ThrowIfFailed (
		device->CreateVertexShader (vsBlob.data(), vsBlob.size(), nullptr, m_VertexShader.ReleaseAndGetAddressOf ())
	);

	auto psBlob = DX::ReadData (pixelPath.c_str ());
	
	ThrowIfFailed (
		device->CreatePixelShader (psBlob.data(), psBlob.size(), nullptr, m_PixelShader.ReleaseAndGetAddressOf ())
	);

	const D3D11_INPUT_ELEMENT_DESC descs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}

	};

	ThrowIfFailed (
		device->CreateInputLayout (descs, _countof(descs), vsBlob.data(), vsBlob.size(), m_InputLayout.ReleaseAndGetAddressOf ())
	);


	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	ThrowIfFailed (
		device->CreateSamplerState (&desc, m_DefaultSampler.ReleaseAndGetAddressOf ())
	);

}

void JohnShader::SetModel(DirectX::SimpleMath::Matrix val)
{
	m_Model = val;
}

void JohnShader::SetView(DirectX::SimpleMath::Matrix val)
{
	m_View = val;
}

void JohnShader::SetProj(DirectX::SimpleMath::Matrix val)
{
	m_Proj = val;
}

void JohnShader::SetEnvironmentTextures(ComPtr<ID3D11ShaderResourceView> diff, ComPtr<ID3D11ShaderResourceView> spec)
{
	m_DiffIBL = diff;
	m_SpecIBL = spec;
}

void JohnShader::SetNumPointLights(int numLights)
{
	m_NumPointLights = numLights;
}

void JohnShader::Recompile()
{
	auto device = Application::Get().GetDevice ();
	std::wstring runtimePath = L"Shaders/Runtime/";


	std::wstring vertPath = m_VertFileSource;
	std::wstring pixelPath = m_PixelFileSource;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* vertShaderBlob = nullptr;
	ID3DBlob* pixelShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = S_OK;
	while(true)
	{


		HRESULT hr = D3DCompileFromFile (
			vertPath.c_str (),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"vs_5_0",
			flags,
			0,
			&vertShaderBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				int msgBoxID = MessageBoxA(Application::Get().GetNativeWindow (),  (char*)errorBlob->GetBufferPointer (), "Compilation of vertex file FAILED", MB_RETRYCANCEL);

				switch (msgBoxID)
				{
				case IDCANCEL:
					Application::Get().RequestExit ();
					break;
				case IDRETRY:
					continue;
					break;
				}

			}
			return;
		}
		
		
		break;
		
	}
	if(FAILED(hr))
	{
		return;
	}
	while(true)
	{
		HRESULT hr = D3DCompileFromFile (
			pixelPath.c_str (),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			"ps_5_0",
			flags,
			0,
			&pixelShaderBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				int msgBoxID = MessageBoxA(Application::Get().GetNativeWindow (), (char*)errorBlob->GetBufferPointer (), "Compilation of pixel shader file FAILED",MB_RETRYCANCEL);

				switch (msgBoxID)
				{
				case IDCANCEL:
					Application::Get().RequestExit ();
					break;
				case IDRETRY:
					continue;
				}

			}
		}
		break;
	}
	ThrowIfFailed (
		device->CreateVertexShader (vertShaderBlob->GetBufferPointer (), vertShaderBlob->GetBufferSize (), nullptr, m_VertexShader.ReleaseAndGetAddressOf ())
	);

	ThrowIfFailed (
		device->CreatePixelShader (pixelShaderBlob->GetBufferPointer (), pixelShaderBlob->GetBufferSize (), nullptr, m_PixelShader.ReleaseAndGetAddressOf ())
	);

}
