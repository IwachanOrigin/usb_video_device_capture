
#include "stdafx.h"
#include "dx11baserenderer.h"
#include <fstream>

using namespace DirectX;
using namespace renderer;

DX11BaseRenderer::DX11BaseRenderer()
  : m_d3dDevice(nullptr)
  , m_immediateContext(nullptr)
  , m_swapchain(nullptr)
  , m_renderTargetView(nullptr)
  , m_srv(nullptr)
  , m_samplerClampLinear(nullptr)
  , m_texture(nullptr)
  , m_cbTexOriginalSize(nullptr)
  , m_renderWidth(0)
  , m_renderHeight(0)
  , m_textureWidth(0)
  , m_textureHeight(0)
{
}

DX11BaseRenderer::~DX11BaseRenderer()
{
}

bool DX11BaseRenderer::init(const HWND hwnd, const uint32_t& width, const uint32_t& height, const uint32_t& fpsNum, const VideoCaptureFormat& vcf)
{
  if (hwnd == nullptr)
  {
    MessageBoxW(nullptr, L"hwnd is NULL.", L"Error", MB_OK);
    return false;
  }

  m_textureWidth = width;
  m_textureHeight = height;

  RECT rc{};
  GetClientRect(hwnd, &rc);
  m_renderWidth = rc.right - rc.left;
  m_renderHeight = rc.bottom - rc.top;

  D3D_DRIVER_TYPE driverTypes[] = {
    D3D_DRIVER_TYPE_HARDWARE
    , D3D_DRIVER_TYPE_WARP
    , D3D_DRIVER_TYPE_REFERENCE
  };

  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  DXGI_SWAP_CHAIN_DESC sd{};
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 1;
  sd.BufferDesc.Width = m_textureWidth;
  sd.BufferDesc.Height = m_textureHeight;
  sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = fpsNum;
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

  // Create a render target view
  ComPtr<ID3D11Texture2D> backBuffer = nullptr;
  hr = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the back buffer from the swap chain.", L"Error", MB_OK);
    return false;
  }

  hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create render target view.", L"Error", MB_OK);
    return false;
  }
  m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

  D3D11_VIEWPORT vp{};
  vp.Width = (FLOAT)sd.BufferDesc.Width;
  vp.Height = (FLOAT)sd.BufferDesc.Height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  m_immediateContext->RSSetViewports(1, &vp);

  // Create the sample state
  D3D11_SAMPLER_DESC sampDesc{};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = m_d3dDevice->CreateSamplerState(&sampDesc, m_samplerClampLinear.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create SamplerState.", L"Error", MB_OK);
    return false;
  }
  m_immediateContext->PSSetSamplers(0, 1, m_samplerClampLinear.GetAddressOf());

  bool result = this->createTexture();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create texture.", L"Error", MB_OK);
    return false;
  }

  result = this->createPipeline(vcf);
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create pipeline.", L"Error", MB_OK);
    return false;
  }

  result = this->createMesh();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create quad mesh.", L"Error", MB_OK);
    return false;
  }

  result = this->createCBTexOrigin();
  if (!result)
  {
    MessageBoxW(nullptr, L"Failed to create ConstantBuffer of texturesize.", L"Error", MB_OK);
    return false;
  }

  return true;
}

bool DX11BaseRenderer::render()
{
  if (!m_immediateContext)
  {
    return false;
  }

  // Clear back buffer
  float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // red,green,blue,alpha
  m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

  {
    // Set the shader's
    m_immediateContext->VSSetConstantBuffers(0, 1, m_cbTexOriginalSize.GetAddressOf());
    m_pipeline.activate();
    if (m_texture && m_immediateContext)
    {
      m_immediateContext->PSSetShaderResources(0, 1, m_srv.GetAddressOf());
    }
    m_quad.activateAndRender();
  }

  // Present swapchain
  if (m_swapchain)
  {
    HRESULT hr = m_swapchain->Present(1, 0);
    if (FAILED(hr))
    {
      MessageBoxW(nullptr, L"Failed to present in Swap Chain.", L"Error", MB_OK);
      return false;
    }
  }
  return true;
}

bool DX11BaseRenderer::createPipeline(const VideoCaptureFormat& vcf)
{
  D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  if (!m_pipeline.create(layout, ARRAYSIZE(layout), m_d3dDevice, m_immediateContext, vcf))
  {
    MessageBoxW(nullptr, L"Failed to create Pipeline.", L"Error", MB_OK);
    return false;
  }

  return true;
}

bool DX11BaseRenderer::createMesh()
{
  struct Vertex {
    float x, y, z;
    float u, v;
  };

  {
    /* texture vertex(x, y, z)
     *              ^
     *              |
     *   -1,1       |        1,1
     *              |
     *              |
     *   ----------0,0------------->
     *              |
     *              |
     *  -1,-1       |        1,-1
     *              |
     *              |
     *
     */

    /* texture color(u, v)
     *
     *           U
     *   0,0-------------> 1,0
     *    |
     *    |
     *    |
     *  V |
     *    |
     *    |
     *    v
     *   1,0               1,1
     */
    SimpleMath::Vector4 white(1, 1, 1, 1);
    std::vector<Vertex> vtxs = {
      { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
      { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
      { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
      {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
      {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
    };
    if (!m_quad.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(Vertex), renderer::eTopology::TRIANGLE_LIST, m_d3dDevice, m_immediateContext))
    {
      MessageBoxW(nullptr, L"Failed to create Quad Mesh.", L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

bool DX11BaseRenderer::createCBTexOrigin()
{
  struct CBTextureSize
  {
    int width;
    int height;
    float padding[2];
  };

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(CBTextureSize);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;

  CBTextureSize texSize{};
  texSize.width = m_textureWidth;
  texSize.height = m_textureHeight;

  D3D11_SUBRESOURCE_DATA subresourceData = {};
  subresourceData.pSysMem = &texSize;

  HRESULT hr = m_d3dDevice->CreateBuffer(&bd, &subresourceData, m_cbTexOriginalSize.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create Constant Buffer.", L"Error", MB_OK);
    return false;
  }
  return true;
}