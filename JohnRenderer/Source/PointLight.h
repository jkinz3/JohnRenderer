#pragma once
class PointLight
{
public:

	PointLight() = default;

	DirectX::SimpleMath::Vector3 GetPosition() const;
	void SetPosition(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetColor() const;
	void SetColor(DirectX::SimpleMath::Vector3 val);
	float GetIntensity() const;
	void SetIntensity(float val);
private:

	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_Color;
	float m_Intensity;


};

