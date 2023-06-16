#pragma once
#include "StepTimer.h"

using namespace DX;
using namespace DirectX;
using namespace DirectX::SimpleMath;


class Node
{
public:
	Node();
	~Node();

	virtual void Update(StepTimer const& timer) = 0;
	virtual Matrix PreDraw(Matrix Model) = 0;
	virtual void Draw(Matrix Model) = 0;
	virtual void CreateDeviceDependentResources() = 0;
	virtual void Initialize() = 0;
	virtual void AddChild(std::shared_ptr<Node> child) = 0;
	virtual void AfterLoad() = 0;
	virtual void IterateThroughChildren(std::function<void(Node&)> func) = 0;
	virtual void IterateThroughChildrenUntil(std::function<bool(Node&)> func) = 0;
	virtual Node* FindChildByIndex(int index) = 0;
	virtual Node* FindChildByID(GUID id) = 0;
	virtual size_t GetNumberOfChildren() = 0;
	virtual std::shared_ptr<Node> GetChild(int i) = 0;
	virtual const std::wstring& GetName() const = 0;
	virtual void SetName(const std::wstring& name) = 0;
	virtual int GetIndex() = 0;
	virtual GUID GetId() = 0;
	virtual bool IsSelected() = 0;
	virtual void SetSelected(bool selected) = 0;
	

};

