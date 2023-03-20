
#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "stdafx.h"

using Microsoft::WRL::ComPtr;

namespace Render
{

class Texture
{
public:
  explicit Texture();
  ~Texture();

  bool create(uint32_t xres, uint32_t yres, DXGI_FORMAT new_format, bool is_dynamic);
  void activate(int slot) const;
  void destroy();
  bool updateFromIYUV(const uint8_t* new_data, size_t data_size);
  bool updateFromYUY2(const uint8_t* new_data, size_t data_size);

private:
  ComPtr<ID3D11Resource> m_texture;
  ComPtr<ID3D11ShaderResourceView> m_srv;
  DXGI_FORMAT m_format;
  uint32_t m_xres;
  uint32_t m_yres;

};

} // Render

#endif // TEXTURE_H_
