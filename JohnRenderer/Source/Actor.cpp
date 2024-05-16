#include "pch.h"
#include "Actor.h"
#include "JohnShader.h"
#include "DepthOnlyShader.h"
#include "SelectionOutlineShader.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;


void Actor::SetMesh(std::shared_ptr<JohnMesh> mesh)
{
	m_Mesh = mesh;
}

std::shared_ptr<JohnMesh> Actor::GetMesh()
{
	return m_Mesh;
}

Vector3 Actor::GetPosition() const
{
	return m_Transform.Position;
}

void Actor::SetPosition(Vector3 val)
{
	m_Transform.Position = val;
	m_Transform.bIsDirty = true;
}

Quaternion Actor::GetRotation() const
{
	return m_Transform.Rotation;
}

void Actor::SetRotation(Quaternion val)
{
	m_Transform.Rotation = val;
	m_Transform.bIsDirty = true;
}

Vector3 Actor::GetScale() const
{
	return m_Transform.Scale;
}

void Actor::SetScale(Vector3 val)
{
	m_Transform.Scale = val;
	m_Transform.bIsDirty = true;
}

Matrix Actor::GetModelMatrix()
{
	return m_Transform.ModelMatrix;

}





void Actor::Draw(std::shared_ptr<JohnShader> inShader)
{

	auto context = Application::Get().GetContext ();

	if(m_Transform.bIsDirty)
	{
		m_Transform.UpdateMatrix ();
	}
	inShader->SetModel (m_Transform.ModelMatrix);
	inShader->Apply ();
	

	m_Mesh->Draw();

}

void Actor::DrawOutline(std::shared_ptr<SelectionOutlineShader> inShader)
{

	auto context = Application::Get().GetContext ();


	Matrix posMat = Matrix::CreateTranslation (m_Transform.Position);
	Matrix rotMat = Matrix::CreateFromQuaternion (m_Transform.Rotation);
	Matrix scaleMat = Matrix::CreateScale (m_Transform.Scale * 1.03f);
	Matrix model = scaleMat * rotMat * posMat;
	inShader->SetModel (model);
	inShader->Apply ();


	m_Mesh->DrawOutline();
}

void Actor::DrawDepth(std::shared_ptr<DepthOnlyShader> inShader)
{
	auto context = Application::Get().GetContext ();

	if (m_Transform.bIsDirty)
	{
		m_Transform.UpdateMatrix ();
	}
	inShader->SetModel (m_Transform.ModelMatrix);
	inShader->Apply ();


	m_Mesh->DrawDepth();
}
