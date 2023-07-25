
#ifndef DX11_YUY2_RENDERER_H_
#define DX11_YUY2_RENDERER_H_

#include "stdafx.h"
#include "dx11baserenderer.h"

using namespace Microsoft::WRL;

namespace renderer
{

struct Constants
{
  int width;
  int height;
  float padding[2];
};

class DX11YUY2Renderer : public DX11BaseRenderer
{
public:
  explicit DX11YUY2Renderer();
  virtual ~DX11YUY2Renderer();

  bool updateTexture(const uint8_t* new_data, size_t data_size) override;

protected:
  bool createTexture() override;

private:
  bool createConstantBuffer();

  ComPtr<ID3D11Buffer> m_constantBuffer;
};

} // renderer

#endif // DX11_YUY2_RENDERER_H_
