
#include "stdafx.h"
#include "dx11nv12renderer.h"

using namespace DirectX;
using namespace renderer;

DX11Nv12Renderer::DX11Nv12Renderer()
  : DX11BaseRenderer()
{
}

DX11Nv12Renderer::~DX11Nv12Renderer()
{
}

bool DX11Nv12Renderer::createTexture()
{
  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = m_textureWidth;
  desc.Height = m_textureHeight * 2;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8_UNORM;
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

bool DX11Nv12Renderer::updateTexture(const uint8_t* new_data, size_t data_size)
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

  // Copy the Y lines
  uint32_t nlines = m_textureHeight;
  uint32_t bytes_per_row = m_textureWidth * bytes_per_texel;
  for (uint32_t y = 0; y < nlines; y++)
  {
    std::memcpy(dst, src, bytes_per_row);
    src += bytes_per_row;
    dst += ms.RowPitch;
  }

  // Now the U and V lines, need to add Width / 2 pixels of padding between each line.
  uint32_t uv_bytes_per_row = bytes_per_row / 2;
  uint32_t srcIndex = 0;
  // U
  for (uint32_t y = 0; y < nlines / 2; y++)
  {
    for (uint32_t x = 0; x < uv_bytes_per_row; x++, srcIndex++)
    {
      *dst = src[srcIndex * 2];
      dst++;
    }
    dst += (ms.RowPitch / 2); // 960
  }
  // V
  srcIndex = 0;
  for (uint32_t y = 0; y < nlines / 2; y++)
  {
    for (uint32_t x = 0; x < uv_bytes_per_row; x++, srcIndex++)
    {
      *dst = src[(srcIndex * 2) + 1];
      dst++;
    }
    dst += (ms.RowPitch / 2); // 960
  }

  m_immediateContext->Unmap(m_texture.Get(), 0);
  return true;
}


