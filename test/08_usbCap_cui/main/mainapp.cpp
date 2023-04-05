
#include "mainapp.h"
#include "dx11base.h"
#include "SimpleMath.h"
#include "dxhelper.h"

using namespace DirectX;
using namespace dx_engine;

MainApp::MainApp()
{
}

MainApp::~MainApp()
{
}

bool MainApp::create(HWND hWnd, int deviceNo, int width, int height, int fps)
{
  if (!DX11Base::getInstance().create(hWnd))
  {
    return false;
  }

  D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  if (!m_pipeVideo.create(layout, ARRAYSIZE(layout)))
  {
    return false;
  }

  struct Vertex {
    float x, y, z;
    float u, v;
  };

  {
    /* texture vertex(x, y, z)
     *              ^
     *              |
     *   -1,1       |        1,1
     *              |
     *              |
     *   ----------0,0------------->
     *              |
     *              |
     *  -1,-1       |        1,-1
     *              |
     *              |
     *
     */

    /* texture color(u, v)
     *
     *           U
     *   0,0-------------> 1,0
     *    |
     *    |
     *    |
     *  V |
     *    |
     *    |
     *    v
     *   1,0               1,1
     */
    SimpleMath::Vector4 white(1, 1, 1, 1);
    std::vector<Vertex> vtxs = {
      { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
      { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
      { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
      {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
      {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
    };
    if (!m_quad.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(Vertex), Render::eTopology::TRIANGLE_LIST))
    {
      return false;
    }
  }

  // INIT
  ThrowIfFailed(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

  CaptureTexture::createAPI();
  // Input device no.
  uint32_t audioDeviceNo = 0;

  if (!m_captureTexture.create(deviceNo, audioDeviceNo, width, height, fps))
  {
    return false;
  }

  return true;
}

void MainApp::render()
{
  DX11Base::getInstance().render();

  {
    // The video
    m_pipeVideo.activate();
    if (m_captureTexture.getTexture())
    {
      m_captureTexture.getTexture()->activate(0);
    }
    m_quad.activateAndRender();
  }

  DX11Base::getInstance().swapChain();
}

void MainApp::update(float dt)
{
  m_captureTexture.update(dt);
}

void MainApp::destroy()
{
  m_captureTexture.destroy();
  CaptureTexture::destroyAPI();
  m_quad.destroy();
  m_pipeVideo.destroy();
  DX11Base::getInstance().destroy();
}
