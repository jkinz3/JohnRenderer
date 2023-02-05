#pragma once
#include "JohnMesh.h"
class JohnPrimitive :
    public JohnMesh
{
public:
	JohnPrimitive();

	size_t GetSize() const;
	void SetSize( size_t val );
	float GetTessellation() const;
	void SetTessellation( float val );

	static John::EAssetType GetStaticType() { return John::EAssetType::JohnPrimitive; }
	virtual John::EAssetType GetAssetType () const override { return GetStaticType (); }

	John::EPrimitiveType GetPrimitiveType() const;
	void SetPrimitiveType( John::EPrimitiveType val );

	void RebuildPrimitive();

private:

	float m_Size = 2;
	size_t m_Tessellation = 3;

	John::EPrimitiveType m_PrimitiveType;

};

