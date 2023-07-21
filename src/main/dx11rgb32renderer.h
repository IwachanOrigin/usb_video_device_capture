
#ifndef DX11_RGB32_RENDERER_H_
#define DX11_RGB32_RENDERER_H_

#include "stdafx.h"
#include "dx11baserenderer.h"

using namespace Microsoft::WRL;

namespace renderer
{

class DX11RGB32Renderer : public DX11BaseRenderer
{
public:
  explicit DX11RGB32Renderer();
  virtual ~DX11RGB32Renderer();

  bool updateTexture(const uint8_t* new_data, size_t data_size) override;

protected:
  bool createTexture() override;

};

} // renderer

#endif // DX11_RGB32_RENDERER_H_
