#ifndef GRAPHICS_H
#define GRAPHICS_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

using DirectX::XMMATRIX;

namespace Garm
{
	namespace System
	{
		namespace Render
		{
			class Graphics
			{
			public:
				Graphics(const HWND& window);
				virtual ~Graphics();

				void Render();
			private:
				// D3D 11 Device, SwapChain, Output interfacing
				ID3D11Device* device;
				ID3D11DeviceContext* immediateContext;
				IDXGISwapChain* swapChain;
				ID3D11RenderTargetView* screenRenderTarget;

				// GBuffer Textures & accessors
				ID3D11Texture2D* texDepthBuffer;
				ID3D11Texture2D* texDiffuse;
				ID3D11Texture2D* texNormal;
				ID3D11RenderTargetView* renderTargetViews[2];
				ID3D11DepthStencilView* dsvDepthBuffer;
				ID3D11ShaderResourceView* srvDiffuse;
				ID3D11ShaderResourceView* srvNormal;
				ID3D11ShaderResourceView* srvDepthBuffer;

				// Render states
				ID3D11DepthStencilState* depthStencilState;
				ID3D11RasterizerState* rasterState;
				D3D11_VIEWPORT viewport;
				XMMATRIX matProjection;
				XMMATRIX matView;

				// Initialization functions
				void createDeviceSwapchain( const HWND& window );
				void createGBuffer(int width, int height);
				void createGBuffer_textures(int width, int height);
				void createGBuffer_views();
				void createRenderState();

				// Shutdown functions
				void releaseRenderState();
				void releaseGBuffer();
				void releaseDeviceSwapchain();
			};
		}
	}
}

#endif