#pragma once
using namespace DirectX;
namespace John
{
	class MouseDeltaTracker
	{
	public:
		void UpdateState( Mouse::State State );
		void EndFrame();

		void GetMouseDelta( int& outX, int& outY ) const
		{
			outX = x;
			outY = y;
		}

	private:
		int x = 0, y = 0;
	};
}

