#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include "Graphics.h"
#include <stdexcept>
#include <fstream>

using Garm::System::Render::Graphics;

Graphics::Graphics( const HWND& window )
{
	createDeviceSwapchain( window );
	createGBuffer( 1024, 768 );
	createRenderState();
	createShader();

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
	releaseShader();
	releaseRenderState();
	releaseGBuffer();
	releaseDeviceSwapchain();
}

void Graphics::Render()
{
	immediateContext->RSSetState( rasterState );
	immediateContext->RSSetViewports( 1, &viewport );
	float clearcolor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	float clearcolor2[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

	// G-Buffer
	immediateContext->OMSetRenderTargets( 2, renderTargetViews, dsvDepthBuffer );
	immediateContext->OMSetDepthStencilState( depthStencilState, 1 );
	immediateContext->ClearDepthStencilView( dsvDepthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	immediateContext->ClearRenderTargetView( renderTargetViews[0], clearcolor2 );
	immediateContext->ClearRenderTargetView( renderTargetViews[1], clearcolor );

	// Screen Output
	immediateContext->OMSetRenderTargets( 1, &screenRenderTarget, 0 );

	immediateContext->IASetInputLayout( composeInputLayout );
	immediateContext->VSSetShader( composeVS, NULL, 0 );
	immediateContext->PSSetShader( composePS, NULL, 0 );

	immediateContext->PSSetSamplers( 0, 1, &composeSampler );
	immediateContext->PSSetShaderResources( 0, 3, shaderResourceView );

	unsigned int stride = sizeof(ComposeVertexType);
	unsigned int offset = 0U;
	immediateContext->IASetVertexBuffers( 0, 1, &fullscreenQuad, &stride, &offset );
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	immediateContext->Draw( 4, 0 );

	ID3D11ShaderResourceView* unbind[3] = { 0, 0, 0 };
	immediateContext->PSSetShaderResources( 0, 3, unbind );

	swapChain->Present( 0, 0 );
}

void Graphics::createDeviceSwapchain( const HWND& window )
{
	HRESULT result = 0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory( &swapChainDesc, sizeof(swapChainDesc) );

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

#if defined(_DEBUG)
	result = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &immediateContext );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create D3D device @" __FILE__ ":" S__LINE__ );
	result = device->QueryInterface( __uuidof(ID3D11Debug), reinterpret_cast<void**>(&dbgDev) );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create D3D debug device @" __FILE__ ":" S__LINE__ );
#else
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &immediateContext );
	if (FAILED( result ))
		throw std::runtime_error( "Unable to create D3D device @" __FILE__ ":" S__LINE__ );
#endif

	ID3D11Texture2D* backBufferPtr;
	result = swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to get SwapChain buffer texture @" __FILE__ ":" S__LINE__ );

	result = device->CreateRenderTargetView( backBufferPtr, NULL, &screenRenderTarget );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create screen render target @" __FILE__ ":" S__LINE__ );

	backBufferPtr->Release();
	backBufferPtr = 0;
}

void Graphics::createGBuffer( int width, int height )
{
	createGBuffer_textures( width, height );
	createGBuffer_views();
}

void Graphics::createGBuffer_textures( int width, int height )
{
	HRESULT result = 0;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory( &textureDesc, sizeof(textureDesc) );

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
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create depth buffer texture @" __FILE__ ":" S__LINE__ );

	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	result = device->CreateTexture2D( &textureDesc, NULL, &texDiffuse );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create diffuse texture @" __FILE__ ":" S__LINE__ );

	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	result = device->CreateTexture2D( &textureDesc, NULL, &texNormal );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create normal texture @" __FILE__ ":" S__LINE__ );
}

void Graphics::createGBuffer_views()
{
	// ==== DepthStencil ====
	HRESULT result = 0;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory( &depthStencilViewDesc, sizeof(depthStencilViewDesc) );

	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView( texDepthBuffer, &depthStencilViewDesc, &dsvDepthBuffer );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create depth stencil view @" __FILE__ ":" S__LINE__ );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof(srvDesc) );

	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texDepthBuffer, &srvDesc, &shaderResourceView[2] );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create depth stencil shader resource view @" __FILE__ ":" S__LINE__ );

	// ==== Normal ====

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory( &rtvDesc, sizeof(rtvDesc) );

	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView( texNormal, &rtvDesc, &renderTargetViews[1] );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create normal render target view @" __FILE__ ":" S__LINE__ );

	ZeroMemory( &srvDesc, sizeof(srvDesc) );

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texNormal, &srvDesc, &shaderResourceView[1] );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create normal shader resource view @" __FILE__ ":" S__LINE__ );

	// ==== Diffuse ====

	ZeroMemory( &rtvDesc, sizeof(rtvDesc) );

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView( texDiffuse, &rtvDesc, &renderTargetViews[0] );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create diffuse render target view @" __FILE__ ":" S__LINE__ );

	ZeroMemory( &srvDesc, sizeof(srvDesc) );

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView( texDiffuse, &srvDesc, &shaderResourceView[0] );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create diffuse shader resource view @" __FILE__ ":" S__LINE__ );
}

void Graphics::createRenderState()
{
	HRESULT result = 0;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory( &depthStencilDesc, sizeof(depthStencilDesc) );

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
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create depth stencil state @" __FILE__ ":" S__LINE__ );

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
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create rasterizer state @" __FILE__ ":" S__LINE__ );
}

void Graphics::createShader()
{
	createShader_scene();
	createShader_compose();
	createShader_fullscreenQuad();
}

void Graphics::createShader_scene()
{
	HRESULT result = 0;

	// Do PS first, since we need VS bytecode for signature generation
	// ==== Scene Pixel Shader ====

	std::ifstream fileStream( L"ScenePS.cso", std::ifstream::binary );
	if ( !fileStream )
		throw std::runtime_error( "Unable to open scene ps shader file @" __FILE__ ":" S__LINE__ );

	fileStream.seekg( 0, std::ifstream::end );
	int fileSize = static_cast<int>(fileStream.tellg());
	fileStream.seekg( 0 );

	std::unique_ptr<char[]> fileContent( new char[fileSize] );
	fileStream.read( fileContent.get(), fileSize );
	fileStream.close();

	result = device->CreatePixelShader( fileContent.get(), fileSize, NULL, &scenePS );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create scene ps @" __FILE__ ":" S__LINE__ );

	// ==== Scene Vertex Shader ====

	fileStream = std::ifstream( L"SceneVS.cso", std::ifstream::binary );
	if ( !fileStream )
		throw std::runtime_error( "Unable to open scene vs shader file @" __FILE__ ":" S__LINE__ );

	fileStream.seekg( 0, std::ifstream::end );
	fileSize = static_cast<int>(fileStream.tellg());
	fileStream.seekg( 0 );

	fileContent = std::unique_ptr<char[]>( new char[fileSize] );
	fileStream.read( fileContent.get(), fileSize );
	fileStream.close();

	result = device->CreateVertexShader( fileContent.get(), fileSize, NULL, &sceneVS );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create scene vs @" __FILE__ ":" S__LINE__ );

	// ==== Vertex Layout ====

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	result = device->CreateInputLayout( polygonLayout, 2, fileContent.get(), fileSize, &sceneInputLayout );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create scene input layout @" __FILE__ ":" S__LINE__ );

}

void Graphics::createShader_compose()
{
	HRESULT result = 0;

	// Do PS first, since we need VS bytecode for signature generation
	// ==== Compose Pixel Shader ====

	std::ifstream fileStream( L"ComposePS.cso", std::ifstream::binary );
	if ( !fileStream )
		throw std::runtime_error( "Unable to open compose ps shader file @" __FILE__ ":" S__LINE__ );

	fileStream.seekg( 0, std::ifstream::end );
	int fileSize = static_cast<int>(fileStream.tellg());
	fileStream.seekg( 0 );

	std::unique_ptr<char[]> fileContent( new char[fileSize] );
	fileStream.read( fileContent.get(), fileSize );
	fileStream.close();

	result = device->CreatePixelShader( fileContent.get(), fileSize, NULL, &composePS );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create compose ps @" __FILE__ ":" S__LINE__ );

	// ==== Compose Vertex Shader ====

	fileStream = std::ifstream( L"ComposeVS.cso", std::ifstream::binary );
	if ( !fileStream )
		throw std::runtime_error( "Unable to open compose vs shader file @" __FILE__ ":" S__LINE__ );

	fileStream.seekg( 0, std::ifstream::end );
	fileSize = static_cast<int>(fileStream.tellg());
	fileStream.seekg( 0 );

	fileContent = std::unique_ptr<char[]>( new char[fileSize] );
	fileStream.read( fileContent.get(), fileSize );
	fileStream.close();

	result = device->CreateVertexShader( fileContent.get(), fileSize, NULL, &composeVS );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create compose vs @" __FILE__ ":" S__LINE__ );

	// ==== Vertex Layout ====

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	result = device->CreateInputLayout( polygonLayout, 2, fileContent.get(), fileSize, &composeInputLayout );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create compose input layout @" __FILE__ ":" S__LINE__ );

}

void Graphics::createShader_fullscreenQuad()
{
	HRESULT result = 0;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(ComposeVertexType)* 4;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	std::unique_ptr<ComposeVertexType[]> vertices( new ComposeVertexType[4] );
	vertices[0].pos = XMFLOAT3( -1.0f, 1.0f, 0.5f );
	vertices[0].tex = XMFLOAT2( 0.0f, 0.0f );
	vertices[1].pos = XMFLOAT3( 1.0f, 1.0f, 0.5f );
	vertices[1].tex = XMFLOAT2( 1.0f, 0.0f );
	vertices[2].pos = XMFLOAT3( -1.0f, -1.0f, 0.5f );
	vertices[2].tex = XMFLOAT2( 0.0f, 1.0f );
	vertices[3].pos = XMFLOAT3( 1.0f, -1.0f, 0.5f );
	vertices[3].tex = XMFLOAT2( 1.0f, 1.0f );

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices.get();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer( &bufferDesc, &vertexData, &fullscreenQuad );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create fullscreen quad vertex buffer @" __FILE__ ":" S__LINE__ );

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState( &samplerDesc, &composeSampler );
	if ( FAILED( result ) )
		throw std::runtime_error( "Unable to create compose texture sampler @" __FILE__ ":" S__LINE__ );
}

void Graphics::renderUpdateState()
{

}

void Graphics::releaseShader()
{
	if (composeSampler)
		composeSampler->Release();
	composeSampler = 0;
	if ( fullscreenQuad )
		fullscreenQuad->Release();
	fullscreenQuad = 0;

	if ( composeInputLayout )
		composeInputLayout->Release();
	composeInputLayout = 0;
	if ( composePS )
		composePS->Release();
	composePS = 0;
	if ( composeVS )
		composeVS->Release();
	composeVS = 0;

	if ( sceneInputLayout )
		sceneInputLayout->Release();
	sceneInputLayout = 0;
	if ( scenePS )
		scenePS->Release();
	scenePS = 0;
	if ( sceneVS )
		sceneVS->Release();
	sceneVS = 0;
}

void Graphics::releaseRenderState()
{
	if ( rasterState )
		rasterState->Release();
	rasterState = 0;
	if ( depthStencilState )
		depthStencilState->Release();
	depthStencilState = 0;
}

void Graphics::releaseGBuffer()
{
	if ( shaderResourceView[0] )
		shaderResourceView[0]->Release();
	shaderResourceView[0] = 0;
	if ( renderTargetViews[0] )
		renderTargetViews[0]->Release();
	renderTargetViews[0] = 0;
	if ( shaderResourceView[1] )
		shaderResourceView[1]->Release();
	shaderResourceView[1] = 0;
	if ( renderTargetViews[1] )
		renderTargetViews[1]->Release();
	renderTargetViews[1] = 0;
	if ( dsvDepthBuffer )
		dsvDepthBuffer->Release();
	dsvDepthBuffer = 0;
	if ( shaderResourceView[2] )
		shaderResourceView[2]->Release();
	shaderResourceView[2] = 0;
	if ( texNormal )
		texNormal->Release();
	texNormal = 0;
	if ( texDiffuse )
		texDiffuse->Release();
	texDiffuse = 0;
	if ( texDepthBuffer )
		texDepthBuffer->Release();
	texDepthBuffer = 0;
}

void Graphics::releaseDeviceSwapchain()
{
	if ( screenRenderTarget )
		screenRenderTarget->Release();
	screenRenderTarget = 0;
	if (immediateContext)
	{
		immediateContext->ClearState();//So objects will get deleted now
		immediateContext->Flush();
		immediateContext->Release();
	}
	immediateContext = 0;
	if ( swapChain )
		swapChain->Release();
	swapChain = 0;

#if defined(_DEBUG)
	if (dbgDev)
	{
		dbgDev->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
		dbgDev->Release();
	}
	dbgDev = 0;
#endif

	if ( device )
		device->Release();
	device = 0;
}