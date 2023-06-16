#include "pch.h"
#include "MeshNode.h"
#include "ShaderMap.h"
MeshNode::MeshNode(int index)
	: ContainerNode(index)
{

}

MeshNode::~MeshNode()
{

}

void MeshNode::CompileAndLoadVertexShader()
{
	ShaderDescriptor descriptor("PBR.hlsl");
	descriptor.AddDefine("NORMALS");

	ID3DBlob* vsBlob = nullptr;

	std::wstring path(L"Shaders/PBR.hlsl");

	
}

void MeshNode::CompileAndLoadPixelShader()
{

}

void MeshNode::Draw(XMMATRIX model)
{

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
