#include "pch.h"
#include "JohnPrimitive.h"
#include "Primitives.h"
JohnPrimitive::JohnPrimitive()
{
	m_PrimitiveType = John::EPrimitiveType::Sphere;
}

size_t JohnPrimitive::GetSize() const
{
	return m_Size;
}

void JohnPrimitive::SetSize( size_t val )
{
	m_Size = val;
	RebuildPrimitive ();
}

float JohnPrimitive::GetTessellation() const
{
	return m_Tessellation;
}

void JohnPrimitive::SetTessellation( float val )
{
	m_Tessellation = val;
	RebuildPrimitive ();
}

John::EPrimitiveType JohnPrimitive::GetPrimitiveType() const
{
	return m_PrimitiveType;
}

void JohnPrimitive::SetPrimitiveType( John::EPrimitiveType val )
{
	m_PrimitiveType = val;
	RebuildPrimitive ();
}

void JohnPrimitive::RebuildPrimitive()
{
	switch(m_PrimitiveType)
	{
	case John::EPrimitiveType::Sphere:
		John::CreateSphereData ( m_Size, m_Tessellation, *GetVertices (), *GetFaces ());
		break;
	}
}
