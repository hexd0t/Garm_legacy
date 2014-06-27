#include "Engine.h"
#include "../Win.h"

using Garm::System::Engine;

Engine::Engine() : World(), Graphics()
{

}

Engine::~Engine()
{
	World.reset();
}

void Engine::Run()
{
	HWND window = Garm::Win32::CreateOutputWindow( L"Garm workbench" );

	Graphics.reset( new Garm::System::Render::Graphics() );

	while (Garm::Win32::GetMessages())
	{
		Graphics->Render();
	}
}