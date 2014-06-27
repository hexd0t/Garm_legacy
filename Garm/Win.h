#ifndef GARM_WIN_H
#define GARM_WIN_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

namespace Garm
{
	namespace Win32
	{
		LRESULT CALLBACK WindowProcedure( HWND window, unsigned int msg, WPARAM wp, LPARAM lp );
		bool GetMessages();
		HWND CreateOutputWindow( const std::wstring& title );
	}
}

#endif