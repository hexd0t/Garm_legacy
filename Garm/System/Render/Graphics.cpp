#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include "Graphics.h"
#include <stdexcept>

using Garm::System::Render::Graphics;

Graphics::Graphics( const HWND& window )
{
	createDeviceSwapchain( window );
	createGBuffer( 1024, 768 );
}

Graphics::~Graphics()
{

}

void Graphics::Render()
{
	immediateContext->OMSetRenderTargets( 1, &screenRenderTarget, 0 );
}

void Graphics::createDeviceSwapchain( const HWND& window )
{
	HRESULT result = 0;
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
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &immediateContext );
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

void Graphics::createGBuffer( int width, int height )
{
	createGBuffer_textures(width, height);
	createGBuffer_views();
}

void Graphics::createGBuffer_textures( int width, int height )
{
	HRESULT result = 0;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory( &textureDesc, sizeof( textureDesc ) );

	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	result = device->CreateTexture2D( &textureDesc, NULL, &texDepthBuffer );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create depth buffer texture" );

	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	result = device->CreateTexture2D( &textureDesc, NULL, &texDiffuse );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create diffuse texture" );

	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	result = device->CreateTexture2D( &textureDesc, NULL, &texNormal );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create normal texture" );
}

void Graphics::createGBuffer_views()
{
	// ==== DepthStencil ====
	HRESULT result = 0;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView( texDepthBuffer, &depthStencilViewDesc, &dsvDepthBuffer );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create depth stencil view" );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof( srvDesc ) );

	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texDepthBuffer, &srvDesc, &srvDepthBuffer );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create depth stencil shader resource view" );

	// ==== Normal ====

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory( &rtvDesc, sizeof( rtvDesc ) );

	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView( texNormal, &rtvDesc, &rtvNormal );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create normal render target view" );

	ZeroMemory( &srvDesc, sizeof( srvDesc ) );

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texNormal, &srvDesc, &srvNormal );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create normal shader resource view" );

	// ==== Diffuse ====

	ZeroMemory( &rtvDesc, sizeof( rtvDesc ) );

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView( texDiffuse, &rtvDesc, &rtvDiffuse );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create diffuse render target view" );

	ZeroMemory( &srvDesc, sizeof( srvDesc ) );

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texDiffuse, &srvDesc, &srvDiffuse );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create diffuse shader resource view" );
}