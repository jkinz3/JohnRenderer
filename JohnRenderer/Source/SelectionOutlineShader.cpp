#include "pch.h"
#include "SelectionOutlineShader.h"

SelectionOutlineShader::SelectionOutlineShader()
	:m_TransformCB(Application::Get().GetDevice ())
{

}

void SelectionOutlineShader::Apply()
{
	auto context = Application::Get ().GetContext ();
	context->IASetInputLayout (m_InputLayout.Get ());
	context->PSSetShader (m_PixelShader.Get(), nullptr, 0);
	context->VSSetShader (m_VertexShader.Get(), nullptr, 0);

	TransformConstants transConst = {};
	Matrix MVP = m_Model * m_View * m_Proj;
	transConst.MVP = MVP.Transpose ();

	m_TransformCB.SetData (context, transConst);

	auto transCB = m_TransformCB.GetBuffer ();
	context->VSSetConstantBuffers (0, 1, &transCB);

}

void SelectionOutlineShader::LoadFromFiles(std::wstring VertFile, std::wstring PixelFile)
{
	std::wstring runtimePath = L"Shaders/Runtime/";

	//strip .cso extension and replace with hlsl

	size_t vertlastDot = VertFile.find_last_of (L".");
	size_t pixellastDot = PixelFile.find_last_of (L".");

	std::wstring vertrawFileName = VertFile.substr (0, vertlastDot);
	std::wstring pixelrawFileName = PixelFile.substr (0, pixellastDot);
	std::wstring hlsl(L".hlsl");
	std::wstring sourceDir(L"Shaders/Source/");
	m_VertFileSource = sourceDir + vertrawFileName + hlsl;
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

	};

	ThrowIfFailed (
		device->CreateInputLayout (descs, _countof(descs), vsBlob.data(), vsBlob.size(), m_InputLayout.ReleaseAndGetAddressOf ())
	);


}

void SelectionOutlineShader::SetModel(DirectX::SimpleMath::Matrix val)
{
	m_Model = val;
}

void SelectionOutlineShader::SetView(DirectX::SimpleMath::Matrix val)
{
	m_View = val;
}

void SelectionOutlineShader::SetProj(DirectX::SimpleMath::Matrix val)
{
	m_Proj = val;
}

