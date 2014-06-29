#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include "Graphics.h"
#include <stdexcept>

using Garm::System::Render::Graphics;

Graphics::Graphics( const HWND& window )
{
	createDeviceSwapchain( window );
	createGBuffer( 1024, 768 );
	createRenderState();

	viewport.Width = 1024.0f;
	viewport.Height = 768.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	matProjection = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PI / 4.0f, 4.0f / 3.0f, 1.0f, 200.0f );
	matView = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet( 0.0f, 1.0f, 1.0f, 1.0f ),
		DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f ),
		DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );
}

Graphics::~Graphics()
{
	releaseGBuffer();
	releaseDeviceSwapchain();
}

void Graphics::Render()
{
	immediateContext->RSSetState( rasterState );
	immediateContext->RSSetViewports( 1, &viewport );
	float clearcolor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };

	// G-Buffer
	immediateContext->OMSetRenderTargets( 2, renderTargetViews, dsvDepthBuffer );
	immediateContext->OMSetDepthStencilState( depthStencilState, 1 );
	immediateContext->ClearDepthStencilView( dsvDepthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	// Screen Output
	immediateContext->OMSetRenderTargets( 1, &screenRenderTarget, 0 );
	immediateContext->ClearRenderTargetView( screenRenderTarget, clearcolor );

	swapChain->Present( 0, 0 );
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

	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	result = device->CreateRenderTargetView( texNormal, &rtvDesc, &renderTargetViews[1] );
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

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView( texDiffuse, &rtvDesc, &renderTargetViews[0] );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create diffuse render target view" );

	ZeroMemory( &srvDesc, sizeof( srvDesc ) );

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texDiffuse, &srvDesc, &srvDiffuse );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create diffuse shader resource view" );
}

void Graphics::createRenderState()
{
	HRESULT result = 0;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	result = device->CreateDepthStencilState( &depthStencilDesc, &depthStencilState );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create depth stencil state" );

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = device->CreateRasterizerState( &rasterDesc, &rasterState );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create rasterizer state" );
}

void Graphics::releaseRenderState()
{
	if (rasterState)
		rasterState->Release();
	rasterState = 0;
	if (depthStencilState)
		depthStencilState->Release();
	depthStencilState = 0;
}

void Graphics::releaseGBuffer()
{
	if (srvDiffuse)
		srvDiffuse->Release();
	srvDiffuse = 0;
	if (renderTargetViews[0])
		renderTargetViews[0]->Release();
	renderTargetViews[0] = 0;
	if (srvNormal)
		srvNormal->Release();
	srvNormal = 0;
	if (renderTargetViews[1])
		renderTargetViews[1]->Release();
	renderTargetViews[1] = 0;
	if (dsvDepthBuffer)
		dsvDepthBuffer->Release();
	dsvDepthBuffer = 0;
	if (srvDepthBuffer)
		srvDepthBuffer->Release();
	srvDepthBuffer = 0;
	if (texNormal)
		texNormal->Release();
	texNormal = 0;
	if (texDiffuse)
		texDiffuse->Release();
	texDiffuse = 0;
	if (texDepthBuffer)
		texDepthBuffer->Release();
	texDepthBuffer = 0;
}

void Graphics::releaseDeviceSwapchain()
{
	if (screenRenderTarget)
		screenRenderTarget->Release();
	screenRenderTarget = 0;
	if (immediateContext)
		immediateContext->Release();
	immediateContext = 0;
	if (swapChain)
		swapChain->Release();
	swapChain = 0;
	if (device)
		device->Release();
	device = 0;
}