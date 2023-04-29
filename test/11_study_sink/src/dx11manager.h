
#ifndef DX11_MANAGER_H_
#define DX11_MANAGER_H_

#include "stdafx.h"
#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Microsoft::WRL;

namespace manager
{

class DX11Manager
{
public:
  static DX11Manager& getInstance();

  bool init(const HWND hwnd);
  ID3D11Texture2D* getBackBuffer() { return m_backBuffer.Get(); }

private:
  ComPtr<ID3D11Device> m_d3dDevice;
  ComPtr<ID3D11DeviceContext> m_immediateContext;
  ComPtr<IDXGISwapChain> m_swapchain;
  ComPtr<ID3D11RenderTargetView> m_renderTargetView;
  ComPtr<ID3D11Texture2D> m_backBuffer;

  explicit DX11Manager();
  ~DX11Manager();
  explicit DX11Manager(const DX11Manager &);
  DX11Manager &operator=(const DX11Manager &);
};

} // manager

#endif // DX11_MANAGER_H_
