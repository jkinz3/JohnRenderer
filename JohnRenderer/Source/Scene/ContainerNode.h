#pragma once
#include "StepTimer.h"
#include "Node.h" 
#include "Components.h"
class ContainerNode :
	public Node
{
public:

	ContainerNode(int index);
	~ContainerNode();

	virtual void Update(StepTimer const& timer);

	virtual void Draw(XMMATRIX model);
	virtual void CreateDeviceDependentResources();
	virtual void Initialize();

	virtual XMMATRIX PreDraw(XMMATRIX model);
	virtual void AfterLoad() override { bLoadingComplete = true; };
	virtual void IterateThroughChildren(std::function<void(Node&)> func) override;
	virtual void IterateThroughChildrenUntil(std::function<bool(Node&)> func) override;
	virtual Node *FindChildByIndex(int index) override;
	virtual Node *FindChildByID(GUID id) override;

	virtual void AddChild(std::shared_ptr<Node> child);
	virtual size_t GetNumberOfChildren() override;
	virtual std::shared_ptr<Node> GetChild(int i) override;
	virtual const std::wstring& GetName() const override;
	virtual void SetName(const std::wstring& name)  override;


	virtual int GetIndex() override { return m_Index; };
	virtual GUID GetId() override { return m_GUID; }

	virtual bool IsSelected() override;
	virtual void SetSelected(bool sel) override;

	virtual void CreateTransform();

	//transformations
	void SetTranslation(Vector3 NewTranslation) { m_Transform.SetTranslation(NewTranslation); }
	void SetScale(Vector3 NewScale) { m_Transform.SetScale (NewScale); }
	void SetRotation(Vector3 InRotation)
	{
		m_Transform.SetRotation (InRotation);

	}
	void SetRotation(Quaternion InRotation)
	{
		m_Transform.SetRotation (InRotation);`

	}

	Vector3 GetTranslation() const { return m_Transform.GetTranslation (); }
	Vector3 GetScale() const { return m_Transform.GetScale (); }
	Quaternion GetRotation() const { return m_Transform.GetRotation (); }
	Vector3 GetRotationEuler() const { return m_Transform.GetRotationEuler(); }

	void ResetTransformations()
	{
		m_Transform.Reset ();
	}
protected:

	Matrix m_Matrix;
	bool bHasMatrix = false;

	int m_Index;

	GUID m_GUID;

	std::vector<std::shared_ptr<Node>> m_Children;
	std::wstring m_Name;

	bool bLoadingComplete = false;
	bool bSelected = false;

	TransformComponent m_Transform;
};

