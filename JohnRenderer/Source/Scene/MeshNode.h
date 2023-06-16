#pragma once
#include "ContainerNode.h"
#include <map>
#include <pplawait.h>
#include <experimental/resumable>
#include <future>




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


};

