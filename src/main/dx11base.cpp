
#include "dx11base.h"
#include "dxhelper.h"

using namespace Render;

DX11Base::DX11Base()
  : m_device(nullptr)
  , m_deviceContext(nullptr)
  , m_rtv(nullptr)
  , m_samplerClampLinear(nullptr)
  , m_renderWidth(0)
  , m_renderHeight(0)
{
}

DX11Base::~DX11Base()
{
}

DX11Base& DX11Base::getInstance()
{
  static DX11Base inst;
  return inst;
}

bool DX11Base::create(HWND hwnd)
{
  HRESULT hr = S_OK;

  RECT rc{};
  GetClientRect(hwnd, &rc);
  m_renderWidth = rc.right - rc.left;
  m_renderHeight = rc.bottom - rc.top;

  D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferCount = 1;
  sd.BufferDesc.Width = m_renderWidth;
  sd.BufferDesc.Height = m_renderHeight;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  hr = D3D11CreateDeviceAndSwapChain(
    nullptr
    , D3D_DRIVER_TYPE_HARDWARE
    , nullptr
    , 0
    , featureLevels
    , numFeatureLevels
    , D3D11_SDK_VERSION
    , &sd
    , &m_swapChain
    , &m_device
    , &featureLevel
    , &m_deviceContext);
  if (FAILED(hr))
  {
    return false;
  }

  // Create a render target view
  ID3D11Texture2D* backbuffer = nullptr;
  hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
  if (FAILED(hr))
  {
    return false;
  }

  hr = m_device->CreateRenderTargetView(backbuffer, nullptr, m_rtv.GetAddressOf());
  backbuffer->Release();
  if (FAILED(hr))
  {
    return false;
  }
  m_deviceContext->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);

  // Setup the viewport
  D3D11_VIEWPORT vp{};
  vp.Width = (FLOAT)m_renderWidth;
  vp.Height = (FLOAT)m_renderHeight;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  m_deviceContext->RSSetViewports(1, &vp);
  
  // Create the sample state
  D3D11_SAMPLER_DESC sampDesc{};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = m_device->CreateSamplerState(&sampDesc, m_samplerClampLinear.GetAddressOf());
  if (FAILED(hr))
  {
    return false;
  }
  m_deviceContext->PSSetSamplers(0, 1, m_samplerClampLinear.GetAddressOf());

  return true;
}

void DX11Base::destroy()
{
  SAFE_RELEASE(m_samplerClampLinear);
  SAFE_RELEASE(m_rtv);
  SAFE_RELEASE(m_swapChain);
  SAFE_RELEASE(m_deviceContext);
  SAFE_RELEASE(m_device);
}

void DX11Base::render()
{
  float clearColor[4] = { 0.0f, 0.0f, 0.2f, 1.0f }; // red,green,blue,alpha
  m_deviceContext->ClearRenderTargetView(m_rtv.Get(), clearColor);
}

void DX11Base::swapChain()
{
  if (m_swapChain)
  {
    m_swapChain->Present(0, 0);
  }
}
