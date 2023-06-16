#include "pch.h"
#include "ContainerNode.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
ContainerNode::ContainerNode(int index)
	:m_Index(index)
{
	CoCreateGuid(&m_GUID);
}

ContainerNode::~ContainerNode()
{

}

void ContainerNode::Update(StepTimer const& timer)
{
	for(auto child : m_Children)
	{
		child->Update(timer);
	}
}

XMMATRIX ContainerNode::PreDraw(XMMATRIX model)
{
	XMMATRIX mat;
	const XMFLOAT3 emptyVector = { 0, 0, 0 };
	if(bHasMatrix)
	{
		mat = m_Matrix;
	}
	else
	{
		XMFLOAT3 scale = m_Transform.Scale;
		XMFLOAT3 trans = m_Transform.Translation;
		XMFLOAT4 rot = m_Transform.GetRotation();

		mat = XMMatrixTranspose(
			XMMatrixAffineTransformation(
				XMLoadFloat3(&m_Transform.Scale),
				XMLoadFloat3(&emptyVector),
				XMLoadFloat4(&m_Transform.Rotation),
				XMLoadFloat3(&m_Transform.Translation)
			));
	}
	if ( !XMMatrixIsIdentity(model) )
	{
		mat = XMMatrixMultiply(model ,mat);
	}
	model = mat;
	return mat;

}
void ContainerNode::Draw(XMMATRIX model)
{
	for(auto child : m_Children)
	{
		auto modelMatrix = child->PreDraw(model);
		child->Draw(modelMatrix);
	}
}

void ContainerNode::CreateDeviceDependentResources()
{
	for(auto child : m_Children)
	{
		child->CreateDeviceDependentResources();
	}
}

void ContainerNode::Initialize()
{

}



void ContainerNode::IterateThroughChildren(std::function<void(Node&)> func)
{
	for(auto child : m_Children)
	{
		func(*child);
		child->IterateThroughChildren(func);
	}
}

void ContainerNode::IterateThroughChildrenUntil(std::function<bool(Node&)> func)
{
	auto result = func(*this);
	if(!result)
	{
		return;
	}
	for(auto child : m_Children)
	{
		result = func(*child);
		if(!result)
		{
			return;
		}
		child->IterateThroughChildrenUntil(func);
	}
}

Node * ContainerNode::FindChildByIndex(int index)
{
	if(GetIndex() == index)
	{
		return this;
	}
	for(auto child : m_Children)
	{
		auto result = child->FindChildByIndex(index);
		if(result != nullptr)
		{
			return result;
		}
	}

	return nullptr;

}

Node * ContainerNode::FindChildByID(GUID id)
{
	if(m_GUID == id)
	{
		return this;
	}
	for(auto child : m_Children)
	{
		return child->FindChildByID(id);
	}
	return nullptr;
}

void ContainerNode::AddChild(std::shared_ptr<Node> child)
{
	m_Children.push_back(child);
}

size_t ContainerNode::GetNumberOfChildren()
{
	return m_Children.size();
}

std::shared_ptr<Node> ContainerNode::GetChild(int i)
{
	return m_Children[i];
}

const std::wstring& ContainerNode::GetName() const
{
	return m_Name;
}

void ContainerNode::SetName(const std::wstring& name)
{
	if(m_Name != name)
	{
		m_Name = name;
	}
}

bool ContainerNode::IsSelected()
{
	return bSelected;
}

void ContainerNode::SetSelected(bool sel)
{
	if(sel == bSelected)
	{
		return;
	}
	IterateThroughChildren([sel](Node& node)
		{
			node.SetSelected(sel);
		}
	);
}

void ContainerNode::CreateTransform(John::Transform data)
{

}
