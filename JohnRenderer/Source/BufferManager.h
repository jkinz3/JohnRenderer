#pragma once
#include "Types.h"

using namespace Microsoft::WRL;
using namespace DX;

template<class T>
class ConstantBufferData
{
public:

	ConstantBufferData(unsigned int slot)
		:m_Slot(slot)
	{

	}
	~ConstantBufferData()
	{
		m_ConstantBuffer.Reset();
	}
	
	void Initialize()
	{
		CD3D11_BUFFER_DESC desc(sizeof(T), D3D11_BIND_CONSTANT_BUFFER);
		auto device = DeviceResources::Get().GetD3DDevice();
		DX::ThrowIfFailed (
			device->CreateBuffer(&desc, nullptr, &m_ConstantBuffer)
		);

	}
	void Release() { m_ConstantBuffer.Reset(); }
	void Update()
	{
		if(m_ConstantBuffer == nullptr)
		{
			return;
		}
		auto context = DeviceResources::Get().GetD3DDeviceContext();
		context->UpdateSubresource1(GetConstantBuffer().Get(), 0, NULL < &(GetBufferData()), 0, 0, 0);

	}
	T& GetBufferData() { return m_BufferData; }
	ComPtr<ID3D11Buffer> GetConstantBuffer() { return m_ConstantBuffer; }
private:
	unsigned int m_Slot;
	T m_BufferData;
	ComPtr<ID3D11Buffer> m_ConstantBuffer;
};

class BufferManager
{
public:
	ConstantBufferData<John::MVPConstantBuffer>& GetMVPBuffer()
	{
		return m_MVPBuffer;
	}

	ConstantBufferData<John::PerFrameConstantBuffer>& GetPerFrameBuffer()
	{
		return m_PerFrameBuffer;
	}

	static BufferManager& Get()
	{
		static BufferManager instance;
		return instance;
	}

protected:
	BufferManager();
private:
	ConstantBufferData<John::MVPConstantBuffer> m_MVPBuffer;
	ConstantBufferData<John::PerFrameConstantBuffer> m_PerFrameBuffer;
};

