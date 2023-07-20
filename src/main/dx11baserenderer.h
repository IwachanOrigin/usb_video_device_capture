
#ifndef DX11_BASE_RENDERER_H_
#define DX11_BASE_RENDERER_H_

#include "stdafx.h"
#include "pipeline.h"
#include "mesh.h"
#include "videocaptureformat.h"

using namespace Microsoft::WRL;

namespace renderer
{

class DX11BaseRenderer
{
public:
  explicit DX11BaseRenderer();
  virtual ~DX11BaseRenderer();

  bool init(const HWND hwnd, const uint32_t& width, const uint32_t& height, const uint32_t& fpsNum, const VideoCaptureFormat& vcf);
  virtual bool updateTexture(const uint8_t* new_data, size_t data_size) = 0;
  bool render();

protected:
  ComPtr<ID3D11Device> m_d3dDevice;
  ComPtr<ID3D11DeviceContext> m_immediateContext;
  ComPtr<ID3D11RenderTargetView> m_renderTargetView;
  ComPtr<IDXGISwapChain> m_swapchain;
  ComPtr<ID3D11SamplerState> m_samplerClampLinear;
  ComPtr<ID3D11ShaderResourceView> m_srv;
  ComPtr<ID3D11Resource> m_texture;
  uint32_t m_renderWidth;
  uint32_t m_renderHeight;
  uint32_t m_textureWidth;
  uint32_t m_textureHeight;
  Pipeline m_pipeline;
  Mesh m_quad;

  virtual bool createTexture() = 0;

private:
  bool createPipeline(const VideoCaptureFormat& vcf);
  bool createMesh();
};

} // renderer

#endif // DX11_BASE_RENDERER_H_
