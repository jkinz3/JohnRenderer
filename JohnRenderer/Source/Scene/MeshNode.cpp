#include "pch.h"
#include "MeshNode.h"
#include "ShaderMap.h"
#include "JohnMesh.h"
#include "BufferManager.h"
MeshNode::MeshNode(int index)
	: ContainerNode(index)
{

}

MeshNode::~MeshNode()
{

}

void MeshNode::CompileAndLoadVertexShader()
{
	ShaderDescriptor descriptor("Shaders/PBRVS.hlsl");

	ID3DBlob* vsBlob = nullptr;

	m_VertShaderWrapper = ShaderMap<VertexShaderWrapper>::Get().FindOrCreateShader(descriptor);



	
}

void MeshNode::CompileAndLoadPixelShader()
{
	ShaderDescriptor descriptor("Shaders/PBRPS.hlsl");

	m_PixelShaderWrapper = ShaderMap<PixelShaderWrapper>::Get().FindOrCreateShader(descriptor);
}

void MeshNode::Draw(XMMATRIX model)
{
	if(!bLoadingComplete)
	{
		return;
	}

	unsigned int indexCount = 0;
	bool indexed = false;

	auto context = DeviceResources::Get().GetD3DDeviceContext();

	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), );

	const UINT Stride = sizeof(Vertex);
	const UINT Offset = 0;
	

	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &Stride, &Offset);
	context->IASetInputLayout(m_VertShaderWrapper->GetInputLayout());

	context->VSSetShader(m_VertShaderWrapper->GetVertexShader(), nullptr, 0);
	context->VSSetConstantBuffers(0, 1, BufferManager::Get().GetMVPBuffer().GetConstantBuffer().GetAddressOf());
	context->PSSetShader(m_PixelShaderWrapper->GetPixelShader(), nullptr, 0);
	context->PSSetConstantBuffers(0, 1, BufferManager::Get().GetPerFrameBuffer().GetConstantBuffer().GetAddressOf());
	context->PSSetConstantBuffers(1, 1, BufferManager::Get().GetObjectBuffer().GetConstantBuffer().GetAddressOf());


	

	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void MeshNode::CreateDeviceDependentResources()
{

}

void MeshNode::Initialize()
{
	ContainerNode::Initialize();
}

void MeshNode::AfterLoad()
{
	CompileAndLoadVertexShader();
	CompileAndLoadPixelShader();
	ContainerNode::AfterLoad();
}

void MeshNode::CreateBuffer()
{

}

void MeshNode::CreateTransform()
{

}
