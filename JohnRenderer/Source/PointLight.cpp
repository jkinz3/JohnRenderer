#include "pch.h"
#include "PointLight.h"

DirectX::SimpleMath::Vector3 PointLight::GetPosition() const
{
	return m_Position;
}

void PointLight::SetPosition(DirectX::SimpleMath::Vector3 val)
{
	m_Position = val;
}

DirectX::SimpleMath::Vector3 PointLight::GetColor() const
{
	return m_Color;
}

void PointLight::SetColor(DirectX::SimpleMath::Vector3 val)
{
	m_Color = val;
}

float PointLight::GetIntensity() const
{
	return m_Intensity;
}

void PointLight::SetIntensity(float val)
{
	m_Intensity = val;
}
