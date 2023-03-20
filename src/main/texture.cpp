
#include "texture.h"
#include "dxhelper.h"
#include "dx11base.h"

using namespace Render;
using namespace dx_engine;

Texture::Texture()
  : m_texture(nullptr)
  , m_srv(nullptr)
  , m_format(DXGI_FORMAT_UNKNOWN)
  , m_xres(0)
  , m_yres(0)
{
}

Texture::~Texture()
{
}

bool Texture::create(uint32_t xres, uint32_t yres, DXGI_FORMAT new_format, bool is_dynamic)
{
  m_xres = xres;
  m_yres = yres;
  m_format = new_format;

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = xres;
  desc.Height = yres;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = new_format;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  if (is_dynamic)
  {
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  }
  
  HRESULT hr = DX11Base::getInstance().getDevice()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)m_texture.GetAddressOf());
  if (FAILED(hr))
  {
    return false;
  }

  // Create a resource view so we can use the data in a shader
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
  srv_desc.Format = new_format;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  hr = DX11Base::getInstance().getDevice()->CreateShaderResourceView(m_texture.Get(), &srv_desc, m_srv.GetAddressOf());
  if (FAILED(hr))
  {
    return false;
  }
  return true;
}

void Texture::activate(int slot) const
{
  DX11Base::getInstance().getDeviceContext()->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
}

void Texture::destroy()
{
  SAFE_RELEASE(m_texture);
  SAFE_RELEASE(m_srv);
}

bool Texture::updateFromIYUV(const uint8_t* new_data, size_t data_size)
{
  if (!new_data)
  {
    return false;
  }

  D3D11_MAPPED_SUBRESOURCE ms{};
  HRESULT hr = DX11Base::getInstance().getDeviceContext()->Map(m_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
  if (FAILED(hr))
  {
    return false;
  }

  uint32_t bytes_per_texel = 1;
  const uint8_t* src = new_data;
  uint8_t* dst = (uint8_t*)ms.pData;

  // Copy the Y lines
  uint32_t nlines = m_yres / 2;
  uint32_t bytes_per_row = m_xres * bytes_per_texel;
  for (uint32_t y = 0; y < nlines; y++)
  {
    memcpy(dst, src, bytes_per_row);
    src += bytes_per_row;
    dst += ms.RowPitch;
  }

  // Now the U and V lines, need to add Width / 2 pixels of padding between each line.
  uint32_t uv_bytes_per_row = bytes_per_row / 2;
  for (uint32_t y = 0; y < nlines; y++)
  {
    memcpy(dst, src, uv_bytes_per_row);
    src += uv_bytes_per_row;
    dst += ms.RowPitch;
  }

  DX11Base::getInstance().getDeviceContext()->Unmap(m_texture.Get(), 0);
  return true;
}

bool Texture::updateFromYUY2(const uint8_t* new_data, size_t data_size)
{
  if (!new_data)
  {
    return false;
  }

  D3D11_MAPPED_SUBRESOURCE ms{};
  HRESULT hr = DX11Base::getInstance().getDeviceContext()->Map(m_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
  if (FAILED(hr))
  {
    return false;
  }

  uint32_t bytes_per_texel = 1;
  const uint8_t* src = new_data;
  uint8_t* dst = (uint8_t*)ms.pData;

  memcpy(dst, src, data_size);

  DX11Base::getInstance().getDeviceContext()->Unmap(m_texture.Get(), 0);
  return true;
}