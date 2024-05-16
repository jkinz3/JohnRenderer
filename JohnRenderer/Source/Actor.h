#pragma once
#include "JohnMesh.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
class JohnShader;
class SelectionOutlineShader;
class DepthOnlyShader;
struct Transform
{
	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;

	Matrix ModelMatrix;

	Transform()
	{
		Position = Vector3::Zero;
		Rotation = Quaternion::Identity;
		Scale = Vector3::One;

		ModelMatrix = Matrix::Identity;

		UpdateMatrix ();
	}
	void UpdateMatrix()
	{
		ModelMatrix = GetLocalMatrix ();
		bIsDirty = false;
	}

	void UpdateMatrix(const Matrix parentMatrix)
	{
		ModelMatrix = GetLocalMatrix () * parentMatrix;
		bIsDirty = false;
	}

	

	Matrix GetLocalMatrix()
	{
		Matrix posMat = Matrix::CreateTranslation (Position);
		Matrix rotMat = Matrix::CreateFromQuaternion (Rotation);
		Matrix scaleMat = Matrix::CreateScale (Scale);

		return scaleMat * rotMat * posMat;

	}

	bool bIsDirty = false;
};

class Actor
{
public:

	void SetMesh(std::shared_ptr<JohnMesh> mesh);

	std::shared_ptr<JohnMesh> GetMesh();

	Vector3 GetPosition() const;
	void SetPosition(Vector3 val);
	Quaternion GetRotation() const;
	void SetRotation(Quaternion val);
	Vector3 GetScale() const;
	void SetScale(Vector3 val);

	Matrix GetModelMatrix();

	Transform GetTransform() const { return m_Transform; }

	void UpdateTransform(Matrix parentMatrix);

	void ForceTransformUpdate(Matrix parentMatrix);

	bool bIsRootNode = false;

	void AddChild(std::shared_ptr<Actor> newChild);


	void Draw(std::shared_ptr<JohnShader> inShader);


	void DrawOutline(std::shared_ptr<SelectionOutlineShader> inShader);

	void DrawDepth(std::shared_ptr<DepthOnlyShader> inShader);


	std::string GetName() const { return m_Name; }
	void SetName(std::string val) { m_Name = val; }
private:


	std::shared_ptr<JohnMesh> m_Mesh;


	Transform m_Transform;

	std::string m_Name;
};

