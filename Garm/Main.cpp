#include "System\Engine.h"
#include <stdexcept>
#include <iostream>

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	try
	{
		std::cout << "Garm Build " __TIMESTAMP__ << std::endl;

		Garm::System::Engine engine;

		engine.Run();

		std::cout << "Press enter to exit" << std::endl;
		std::cin.get();
	}
	catch (std::exception& ex)
	{
		std::cerr << std::endl << "Unhandled exception: " << ex.what() << std::endl;
		std::cout << "Press enter to exit" << std::endl;
		std::cin.get();
	}
	catch (...)
	{
		std::cerr << std::endl << "Unhandled unknown exception!" << std::endl;
		std::cout << "Press enter to exit" << std::endl;
		std::cin.get();
	}
}