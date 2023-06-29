#pragma once
#include "ContainerNode.h"
#include <map>
#include <pplawait.h>
#include <experimental/resumable>
#include <future>
#include "ShaderMap.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;


class MeshNode : public ContainerNode
{
public:
	MeshNode(int index);
	~MeshNode();

	void CompileAndLoadVertexShader();
	void CompileAndLoadPixelShader();

	virtual void Draw(XMMATRIX model);
	virtual void CreateDeviceDependentResources();
	virtual void Initialize();
	virtual void AfterLoad();

	void CreateBuffer();
	void CreateTransform();

	std::shared_ptr<VertexShaderWrapper> m_VertShaderWrapper;
	std::shared_ptr<PixelShaderWrapper> m_PixelShaderWrapper;

	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11Buffer> m_IndexBuffer;

	size_t m_IndexCount;

};

