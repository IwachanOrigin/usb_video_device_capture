
#include "stdafx.h"
#include "dx11yuy2renderer.h"

using namespace DirectX;
using namespace renderer;

DX11YUY2Renderer::DX11YUY2Renderer()
  : DX11BaseRenderer()
  , m_constantBuffer(nullptr)
{
}

DX11YUY2Renderer::~DX11YUY2Renderer()
{
}

bool DX11YUY2Renderer::createTexture()
{
  if (!this->createConstantBuffer())
  {
    MessageBoxW(nullptr, L"Failed to create Constant buffer.", L"Error", MB_OK);
    return false;
  }

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = m_textureWidth;
  desc.Height = m_textureHeight;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  HRESULT hr = m_d3dDevice->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)m_texture.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create ID3D11Texture2D.", L"Error", MB_OK);
    return false;
  }

  // Create a resource view so we can use the data in a shader
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
  srv_desc.Format = desc.Format;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  hr = m_d3dDevice->CreateShaderResourceView(m_texture.Get(), &srv_desc, m_srv.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create shader resource view.", L"Error", MB_OK);
    return false;
  }
  return true;
}

bool DX11YUY2Renderer::updateTexture(const uint8_t* new_data, size_t data_size)
{
  if (!new_data)
  {
    return false;
  }

  if (!m_immediateContext)
  {
    return false;
  }

  Constants constants;
  constants.width = m_textureWidth;
  constants.height = m_textureHeight;

  // Set constant buffer
  if (m_constantBuffer)
  {
    m_immediateContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &constants, 0, 0);
    m_immediateContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
  }

  D3D11_MAPPED_SUBRESOURCE ms{};
  HRESULT hr = m_immediateContext->Map(m_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
  if (FAILED(hr))
  {
    return false;
  }

  uint32_t bytes_per_texel = 1;
  const uint8_t* src = new_data;
  uint8_t* dst = (uint8_t*)ms.pData;

  std::memcpy(dst, src, data_size);

  m_immediateContext->Unmap(m_texture.Get(), 0);
  return true;
}

bool DX11YUY2Renderer::createConstantBuffer()
{
  D3D11_BUFFER_DESC bd = {0};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(Constants);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;

  HRESULT hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_constantBuffer);
  if (FAILED(hr))
  {
    return false;
  }
  return true;
}
