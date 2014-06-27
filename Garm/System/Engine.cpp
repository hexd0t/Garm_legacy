#include "Engine.h"
#include "../Win.h"

using Garm::System::Engine;

Engine::Engine() : World(new Garm::Data::World())
{

}

Engine::~Engine()
{
	World.reset();
}

void Engine::Run()
{
	HWND window = Garm::Win32::CreateOutputWindow( L"Garm workbench" );
	while (Garm::Win32::GetMessages())
	{

	}
}