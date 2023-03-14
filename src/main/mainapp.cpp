
#include "mainapp.h"
#include "dx11base.h"
#include "SimpleMath.h"
#include "dxhelper.h"

using namespace DirectX;
using namespace dx_engine;

MainApp::MainApp()
  : m_captureTexture(nullptr)
{
}

MainApp::~MainApp()
{
}

bool MainApp::create(HWND hWnd)
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
  ThrowIfFailed(MFStartup(MF_VERSION));

  // Get devices.
  uint32_t deviceCount = 0;
  IMFActivate** devices = nullptr;
  {
    std::shared_ptr<IMFAttributes> pAttributes;
    IMFAttributes* pRawAttributes = nullptr;
    ThrowIfFailed(MFCreateAttributes(&pRawAttributes, 1));
    pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

    ThrowIfFailed(pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));

    ThrowIfFailed(MFEnumDeviceSources(pAttributes.get(), &devices, &deviceCount));
  }

  HRESULT hr = CaptureTexture::createInst(&m_captureTexture);
  if (FAILED(hr))
  {
    return false;
  }

  // Input device no.
  uint32_t selectionNo = 1;
  HPOWERNOTIFY hPowerNotify = nullptr;
  HPOWERNOTIFY hPowerNotifyMonitor = nullptr;
  SYSTEM_POWER_CAPABILITIES pwrCaps;

  ThrowIfFailed(m_captureTexture->initCaptureTexture(devices[selectionNo]));
  devices[selectionNo]->AddRef();

  // Information cannot be obtained from the device without the following process.
  hPowerNotify = RegisterSuspendResumeNotification((HANDLE)hWnd, DEVICE_NOTIFY_WINDOW_HANDLE);
  hPowerNotifyMonitor = RegisterPowerSettingNotification((HANDLE)hWnd, &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);
  ZeroMemory(&pwrCaps, sizeof(pwrCaps));
  GetPwrCapabilities(&pwrCaps);

  // Start preview
  ThrowIfFailed(m_captureTexture->startPreview());

  return true;
}

void MainApp::render()
{
  if (m_captureTexture->getSamplerCallback()->getTexture() == nullptr)
  {
    return;
  }

  DX11Base::getInstance().render();

  {
    // The video
    m_pipeVideo.activate();
    m_captureTexture->getSamplerCallback()->getTexture()->activate(0);
    m_quad.activateAndRender();
  }

  DX11Base::getInstance().swapChain();
}

void MainApp::update(float dt)
{
  m_captureTexture->update(dt);
}

void MainApp::destroy()
{
  m_captureTexture->stopPreview();
  m_captureTexture->destroyCapEngine();
  m_quad.destroy();
  m_pipeVideo.destroy();
  DX11Base::getInstance().destroy();
}
