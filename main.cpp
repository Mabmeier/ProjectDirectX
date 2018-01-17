#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <stdio.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include "camera.h"
#include "objfilemodel.h"
#include "Model.h"
#include <xnamath.h>
int (WINAPIV * __vsnprintf_s)(char *, size_t, const char*, va_list) = _vsnprintf;

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pBackBufferRTView = NULL;
ID3D11Buffer*	g_pVertexBuffer;
ID3D11VertexShader*	g_pVertexShader;
ID3D11PixelShader*	g_pPixelShader;
ID3D11InputLayout* g_pInputLayout;
ID3D11Buffer*	   g_pConstantBuffer0;
ID3D11Buffer*	   g_pConstantBuffer1;
ID3D11ShaderResourceView* g_pTexture0;		// Declared with other global variables at start of main()
ID3D11SamplerState*		g_pSampler0;
ID3D11DepthStencilView* g_pZBuffer;
XMVECTOR	g_directional_light_shines_from;
XMVECTOR	g_directional_light_colour;
XMVECTOR	g_ambient_light_colour;

Camera*				g_pCamera = NULL;
Model*				g_pModel = NULL;
int counter = 0;
float zRotaVariable = 0;
float degrees = 0;
//Define vertex structure
struct POS_COL_TEX_NORM_VERTEX
{
	XMFLOAT3 Pos;
	XMFLOAT4 Col;
	XMFLOAT2 Texture0;
	XMFLOAT3 Normal;
};

struct CONSTANT_BUFFER0
{
	XMMATRIX WorldViewProjection;
	XMVECTOR directional_light_vector;
	XMVECTOR directional_light_colour;
	XMVECTOR ambient_light_colour;
	//float RedAmount;	// 4 Bytes
	//XMFLOAT3 packing_bytes;
	//float scale;
};	// TOTAL SIZE = 112 bytes

	// Rename for each tutorial
char		g_TutorialName[100] = "Tutorial 07 Exercise 01\0";


//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
HRESULT InitialiseGraphics(void);
void ShutdownD3D();
void RenderFrame(void);

//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}
	if (FAILED(InitialiseGraphics()))	// 03-01
	{
		DXTRACE_MSG("Failed to initialise graphics");
		return 0;
	}
	// Main message loop
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RenderFrame();
		}
	}
	ShutdownD3D();
	return (int)msg.wParam;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "Me Window\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//	.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(g_hWnd);
		if (wParam == VK_LEFT) {
			g_pCamera->LeftRight(-5);
		}
		if (wParam == VK_RIGHT) {
			g_pCamera->LeftRight(5);
		}
		if (wParam == VK_UP) {
			g_pCamera->Forward(5);
		}
		if (wParam == VK_DOWN) {
			g_pCamera->Forward(-5);
		}
		if (wParam == 0x51) {
			g_pCamera->Rotate(-5);
		}
		if (wParam == 0x45) {
			g_pCamera->Rotate(5);
		}
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&g_pBackBufferRTView);
	pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	//Create a Z buffer texture;
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));
	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D * pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if (FAILED(hr))return hr;

	// Create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();

	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &viewport);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_pVertexBuffer) g_pVertexBuffer->Release();//03-01
	if (g_pInputLayout)	g_pInputLayout->Release();//03-01
	if (g_pVertexShader) g_pVertexShader->Release();//03-01
	if (g_pPixelShader) g_pPixelShader->Release();//03-01

	if (g_pTexture0) g_pTexture0->Release();
	if (g_pSampler0) g_pSampler0->Release();
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pConstantBuffer0)g_pConstantBuffer0->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
	if (g_pCamera) g_pCamera->~Camera();

}

//////////////////////////////////////////////////////////////////////////////////////
// Init graphics
//////////////////////////////////////////////////////////////////////////////////////
HRESULT	InitialiseGraphics()//03-01
{
	HRESULT hr = S_OK;
	g_pModel = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pModel->LoadObjModel("assets/Sphere.obj");
	// Define vertices of a triangle - screen coordinates -1.0f to +1.0f


	//D3DX11CreateShaderResourceViewFromFile(g_pD3DDevice, "assets/font2.bmp", NULL, NULL, &g_pTexture0, NULL);

	g_pCamera = new Camera(0.0f, 0.0f, -0.5f, 0.0f);
	//g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	//D3D11_SAMPLER_DESC sampler_desc;
	//ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	//sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	//
	//g_pD3DDevice->CreateSamplerState(&sampler_desc, &g_pSampler0);

	return S_OK;

}

// Render frame
void RenderFrame(void)
{
	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.9f, 1.0f, 1.0f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//// Set vertex buffer // 03-01
	//UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);
	//UINT offset = 0;
	//g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	g_directional_light_shines_from = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
	g_directional_light_colour = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	g_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);

	CONSTANT_BUFFER0 cb0_values, cb1_values;
	//cb0_values.RedAmount = 0.5f;	// 50% of vertex red value

	/* MATRICES */
	XMMATRIX  world, worldTwo;
	XMMATRIX view;
	XMMATRIX projection;
	worldTwo = XMMatrixTranslation(0, 1, 9);
	world = XMMatrixTranslation(counter, 0, 5);

	//XMMATRIX transpose;
	//
	//transpose = XMMatrixTranspose(world);	//model world matrix
	//cb0_values.directional_light_colour = g_directional_light_colour;
	//cb0_values.ambient_light_colour = g_ambient_light_colour;
	//cb0_values.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	//cb0_values.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);
	//
	////world = XMMatrixTranslation(2, 0, 10);
	////world *= XMMatrixRotationZ(XMConvertToRadians(15));
	////world = XMMatrixRotationX(XMConvertToRadians(degrees));
	////world *= XMMatrixTranslation(0, 0, 5);
	//
	//worldTwo *= XMMatrixRotationZ(XMConvertToRadians(zRotaVariable));

	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0 / 480.0, 1.0, 100.0);
	//set view with camera values
	view = XMMatrixIdentity();
	view = g_pCamera->GetViewMatrix();
	// set WorldViewProjection
	//cb0_values.WorldViewProjection = world * view * projection;

	//upload the new values for the constant buffer
	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);
	//// set constant buffer to active
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);

	//Select which primitive type to use //03-01
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//g_pImmediateContext->PSSetSamplers(0, 1, &g_pSampler0);
	//g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture0);

	//Draw the vertex buffer to the back buffer //03-01
	//g_pImmediateContext->Draw(36, 0);

	//cb1_values.WorldViewProjection = worldTwo * view * projection;

	//upload the new values for the constant buffer
	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb1_values, 0, 0);

	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer0);
	//g_pImmediateContext->Draw(36, 0);

	g_pModel->SetZPos(20);

	g_pModel->Draw(&view, &projection);


	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}