#include "pch.h"
#include "Application.h"


int main(int argc, char* argv[])
{

	if(Application::Get().Initialize ())
	{
		try
		{
		Application::Get().Run ();

		}
		catch(const std::exception&)
		{
			return EXIT_FAILURE;
		}
	}

	Application::Get().CleanupApplication ();
	return 0;
}