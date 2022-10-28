#include "pch.h"
#include "MouseDeltaTracker.h"

void John::MouseDeltaTracker::UpdateState( Mouse::State State )
{
	x += State.x;
	y += State.y;

}

void John::MouseDeltaTracker::EndFrame()
{
	x = 0;
	y = 0;
}
