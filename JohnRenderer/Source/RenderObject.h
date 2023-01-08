#pragma once
#include "Types.h"

class Material;
class JohnMesh;

class RenderObject
{
public:

	Matrix GetTransformationMatrix();

	void Draw( ID3D11DeviceContext* context );

	John::Transform GetTransform() const;
	void SetTransform( John::Transform val );

	Vector3 GetPosition() const;
	void SetPosition( Vector3 val );
	Quaternion GetRotation() const;
	void SetRotation( Quaternion val );
	Vector3 GetScale() const;
	void SetScale( Vector3 val );

	Vector3 GetRotationEuler() const;
	void SetRotationEuler( Vector3 NewEuler );

	void ResetTransformations();

	std::shared_ptr<Material> GetMaterial() const;
	void SetMaterial( std::shared_ptr<Material> val );
	std::shared_ptr<JohnMesh> GetMesh() const;
	void SetMesh( std::shared_ptr<JohnMesh> val );


	std::string GetName() const;
	void SetName( std::string val );
private:

		John::Transform m_Transform;

		std::shared_ptr<Material> m_Material;
		std::shared_ptr<JohnMesh> m_Mesh;

		std::string m_Name = std::string( "Model" );

public:

};

