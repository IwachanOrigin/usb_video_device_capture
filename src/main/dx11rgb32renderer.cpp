
#include "stdafx.h"
#include "dx11rgb32renderer.h"

using namespace DirectX;
using namespace renderer;

DX11RGB32Renderer::DX11RGB32Renderer()
  : DX11BaseRenderer()
{
}

DX11RGB32Renderer::~DX11RGB32Renderer()
{
}

bool DX11RGB32Renderer::createTexture()
{
  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = m_textureWidth;
  desc.Height = m_textureHeight;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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

bool DX11RGB32Renderer::updateTexture(const uint8_t* new_data, size_t data_size)
{
  if (!new_data)
  {
    return false;
  }

  if (!m_immediateContext)
  {
    return false;
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


