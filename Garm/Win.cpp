#include "Win.h"
#include <stdexcept>
#include "System/Engine.h"

Garm::System::Engine* Engine;

LRESULT CALLBACK Garm::Win32::WindowProcedure( HWND window, unsigned int msg, WPARAM wp, LPARAM lp )
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage( 0 );
		return 0L;
	default:
		return DefWindowProc( window, msg, wp, lp );
	}
}

bool Garm::Win32::GetMessages()
{
	MSG msg;
	while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) > 0)
	{
		//TranslateMessage( &uMsg ); Only needed for text input (KeyDown -> Character messages)
		DispatchMessage( &msg );
		if (msg.message == WM_QUIT)
			return false;
	}
	return true;
}

HWND Garm::Win32::CreateOutputWindow( const std::wstring& title )
{
	const wchar_t* const myclass = L"outputwindow";
	WNDCLASSEX wndclass = {
		sizeof( WNDCLASSEX ),
		CS_DBLCLKS,
		Garm::Win32::WindowProcedure,
		0,
		0,
		GetModuleHandle( 0 ),
		LoadIcon( 0, IDI_APPLICATION ),
		LoadCursor( 0, IDC_ARROW ),
		0, //HBRUSH( COLOR_WINDOW + 1 ), // ToDo: test 0
		0,
		myclass,
		LoadIcon( 0, IDI_APPLICATION )
	};
	if (!RegisterClassEx( &wndclass ))
		throw std::runtime_error( "Unable to register window class" );
	
	HWND window = CreateWindowEx(
		0,
		myclass,
		title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1024,
		768,
		0,
		0,
		GetModuleHandle( 0 ),
		0 );

	if (!window)
		throw std::runtime_error( "Unable to create window" );

	ShowWindow( window, SW_SHOW );
	SetForegroundWindow( window );
	SetFocus( window );
	
	return window;
}