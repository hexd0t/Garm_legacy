#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include "Graphics.h"
#include <stdexcept>

using Garm::System::Render::Graphics;

Graphics::Graphics( const HWND& window )
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = 1024;
	swapChainDesc.BufferDesc.Height = 768;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;//Disable vsync
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.SampleDesc.Count = 1;//Disable MSAA
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	result = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, immediateContext );
	if (FAILED( result ))
			throw std::runtime_error( "Unable to create D3D device" );

	ID3D11Texture2D* backBufferPtr;
	result = swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to get SwapChain buffer texture" );

	result = device->CreateRenderTargetView( backBufferPtr, NULL, &screenRenderTarget );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create screen render target" );

	backBufferPtr->Release();
	backBufferPtr = 0;
}

Graphics::~Graphics()
{

}

void Graphics::Render()
{
	immediateContext->OMSetRenderTargets( 1, &screenRenderTarget, 0 );
}