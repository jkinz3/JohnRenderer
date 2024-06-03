#pragma once
struct AABB
{
	DirectX::SimpleMath::Vector3 m_Min;
	DirectX::SimpleMath::Vector3 m_Max;

	AABB()
	{
		m_Min = DirectX::SimpleMath::Vector3::Zero;
		m_Max = DirectX::SimpleMath::Vector3::Zero;
	}

	AABB(const DirectX::SimpleMath::Vector3& min, const DirectX::SimpleMath::Vector3& max)
		:m_Min(min),  m_Max(max)
	{

	}

	~AABB()
	{

	}
};