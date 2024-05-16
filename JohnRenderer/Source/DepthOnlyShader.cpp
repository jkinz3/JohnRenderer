#include "pch.h"
#include "DepthOnlyShader.h"

DepthOnlyShader::DepthOnlyShader()
	: m_TransformCB(Application::Get().GetDevice ())
{

}

void DepthOnlyShader::Apply()
{
	auto context = Application::Get ().GetContext ();
	context->IASetInputLayout (m_InputLayout.Get ());
	context->VSSetShader (m_VertexShader.Get(), nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	TransformConstants transConst = {};
	Matrix MVP = m_Model * m_View * m_Proj;
	transConst.MVP = MVP.Transpose ();
	transConst.Model = m_Model.Transpose ();

	m_TransformCB.SetData (context, transConst);

	auto transCB = m_TransformCB.GetBuffer ();
	context->VSSetConstantBuffers (0, 1, &transCB);





}

void DepthOnlyShader::LoadFromFiles(std::wstring VertFile)
{
	std::wstring runtimePath = L"Shaders/Runtime/";

	size_t vertLastDot = VertFile.find_last_of(L".");
	
	std::wstring vertRawName = VertFile.substr (0, vertLastDot);
	std::wstring hlsl(L".hlsl");
	std::wstring sourceDir(L"Shaders/Source/");
	m_VertFileSource = sourceDir + vertRawName + hlsl;

	std::wstring vertPath = runtimePath + VertFile;

	auto device = Application::Get().GetDevice ();

	auto blob = DX::ReadData(vertPath.c_str ());

	ThrowIfFailed (
		device->CreateVertexShader (blob.data(), blob.size(), nullptr, m_VertexShader.ReleaseAndGetAddressOf ())
	);

	const D3D11_INPUT_ELEMENT_DESC descs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}

	};

	ThrowIfFailed (
		device->CreateInputLayout (descs, _countof(descs), blob.data(), blob.size(), m_InputLayout.ReleaseAndGetAddressOf ())
	);
}

void DepthOnlyShader::SetModel(DirectX::SimpleMath::Matrix val)
{
	m_Model = val;
}

void DepthOnlyShader::SetView(DirectX::SimpleMath::Matrix val)
{
	m_View = val;
}

void DepthOnlyShader::SetProj(DirectX::SimpleMath::Matrix val)
{
	m_Proj = val;
}
void DepthOnlyShader::Recompile()
{

}
