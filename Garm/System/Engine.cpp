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

	Graphics = shared_ptr<Garm::System::Render::Graphics>( new Garm::System::Render::Graphics( window ) );

	while (Garm::Win32::GetMessages())
	{
		Graphics->Render();
	}
}