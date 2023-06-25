#pragma once
#include "ContainerNode.h"
#include <map>
#include <pplawait.h>
#include <experimental/resumable>
#include <future>

#include "ShaderMap.h"


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

private:

	class BufferWrapper
	{
	public:
		BufferWrapper(ComPtr<ID3D11Buffer> buffer)
			:m_Buffer(buffer)
		{

		}
		BufferWrapper()
		{

		}

		ComPtr<ID3D11Buffer>& GetBuffer() { return m_Buffer; }
	private:
		ComPtr<ID3D11Buffer> m_Buffer;
	};

	std::map<std::wstring, BufferWrapper> m_Buffers;

	size_t m_IndexCount;

	std::shared_ptr<VertexShaderWrapper> m_VertexShaderWrapper;
	std::shared_ptr<PixelShaderWrapper> m_PixelShaderWrapper;

};

