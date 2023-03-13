
#ifndef DX11_BASE_H_
#define DX11_BASE_H_

#include "stdafx.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Render
{

class DX11Base
{
public:
  static DX11Base& getInstance();

  bool create(HWND hwnd);
  void destroy();
  void render();
  void swapChain();

  ComPtr<ID3D11Device> getDevice() { return m_device; }
  ComPtr<ID3D11DeviceContext> getDeviceContext() { return m_deviceContext; }

private:
  explicit DX11Base();
  ~DX11Base();
  explicit DX11Base(const DX11Base&);
  DX11Base& operator=(const DX11Base&);

  ComPtr<ID3D11Device> m_device;
  ComPtr<ID3D11DeviceContext> m_deviceContext;
  ComPtr<ID3D11RenderTargetView> m_rtv;
  ComPtr<IDXGISwapChain> m_swapChain;
  ComPtr<ID3D11SamplerState> m_samplerClampLinear;
  uint32_t m_renderWidth;
  uint32_t m_renderHeight;
};

} // Render

#endif // DX11_BASE_H_
