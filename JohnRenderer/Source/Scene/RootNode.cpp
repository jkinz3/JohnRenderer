#include "pch.h"
#include "RootNode.h"

RootNode::RootNode(int index)
	:ContainerNode(index)
{
	m_Name = std::wstring(L"SceneRoot");
}

RootNode::~RootNode()
{

}
