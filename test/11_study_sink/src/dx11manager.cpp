
#include "stdafx.h"
#include "dx11manager.h"

using namespace manager;

DX11Manager::DX11Manager()
{
}

DX11Manager::~DX11Manager()
{
}

DX11Manager& DX11Manager::getInstance()
{
  static DX11Manager inst;
  return inst;
}

bool DX11Manager::init(const HWND hwnd)
{
   D3D_DRIVER_TYPE driverTypes[] = {
    D3D_DRIVER_TYPE_HARDWARE
    , D3D_DRIVER_TYPE_WARP
    , D3D_DRIVER_TYPE_REFERENCE
  };

  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  DXGI_SWAP_CHAIN_DESC sd{};
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 1;
  sd.BufferDesc.Width = 800;
  sd.BufferDesc.Height = 600;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.OutputWindow = hwnd;
  sd.Windowed = true;

  D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_11_1
    , D3D_FEATURE_LEVEL_11_0
    , D3D_FEATURE_LEVEL_10_1
    , D3D_FEATURE_LEVEL_10_0
  };

  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

  HRESULT hr = E_FAIL;
  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
  {
    driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDeviceAndSwapChain(
      nullptr
      , driverType
      , nullptr
      , 0
      , featureLevels
      , numFeatureLevels
      , D3D11_SDK_VERSION
      , &sd
      , &m_swapchain
      , &m_d3dDevice
      , &featureLevel
      , &m_immediateContext);

    if (SUCCEEDED(hr))
    {
      break;
    }
  }

  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create DirectX Device and SwapChain.", L"Error", MB_OK);
    return false;
  }

  hr = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_backBuffer);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the back buffer from the swap chain.", L"Error", MB_OK);
    return false;
  }

  m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

  D3D11_VIEWPORT vp{};
  vp.Width = 800;
  vp.Height = 600;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  m_immediateContext->RSSetViewports(1, &vp);

  return true;
}
