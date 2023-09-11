
#include "videocapturemanager.h"
#include "videocapturecallback.h"
#include "videocapturecallbackdmo.h"
#include "videocapturecallbackshader.h"
#include "dx11nv12renderer.h"
#include "dx11yuy2renderer.h"
#include "dx11rgb32renderer.h"

using namespace renderer;

VideoCaptureManager::VideoCaptureManager()
  : m_sourceReader(nullptr)
  , m_wcSymbolicLink(nullptr)
  , m_videoCaptureCB(nullptr)
  , m_capWidth(0)
  , m_capHeight(0)
  , m_capFps(0)
  , m_vcf(VideoCaptureFormat::VideoCapFmt_NONE)
  , m_renderer(nullptr)
{
}

VideoCaptureManager::~VideoCaptureManager()
{
}

VideoCaptureManager& VideoCaptureManager::getInstance()
{
  static VideoCaptureManager inst;
  return inst;
}

int VideoCaptureManager::init(IMFActivate *pActivate, HWND previewWnd, VideoCaptureColorConvMode vcccm)
{
  ComPtr<IMFMediaSource> mediaSource = nullptr;
  ComPtr<IMFAttributes> attributes = nullptr;
  ComPtr<IMFMediaType> mediaType = nullptr;
  HRESULT hr = S_OK;

  hr = pActivate->ActivateObject(__uuidof(IMFMediaSource), (void**)mediaSource.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to activate object.", L"Error", MB_OK);
    return -1;
  }

  UINT32 sizeSymbolicLink = 0;
  hr = pActivate->GetAllocatedString(
    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK
    , &m_wcSymbolicLink
    , &sizeSymbolicLink);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get symbolic link.", L"Error", MB_OK);
    return -1;
  }

  hr = MFCreateAttributes(attributes.GetAddressOf(), 1);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create attributes", L"Error", MB_OK);
    return -1;
  }

  m_videoCaptureCB = this->callbackFactory(vcccm);
  if (!m_videoCaptureCB)
  {
    MessageBoxW(nullptr, L"Failed to generate callback class.", L"Error", MB_OK);
    return -1;
  }

  hr = attributes->SetUnknown(
    MF_SOURCE_READER_ASYNC_CALLBACK
    , m_videoCaptureCB.Get()
    );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set callback.", L"Error", MB_OK);
    return -1;
  }

  hr = MFCreateSourceReaderFromMediaSource(
    mediaSource.Get()
    , attributes.Get()
    , m_sourceReader.GetAddressOf()
    );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create source reader from media source.", L"Error", MB_OK);
    return -1;
  }

  // Set the source reader.
  // And capture width, capture height, capture fps are setted to internal variables in this function.
  hr = m_videoCaptureCB->setSourceReader(m_sourceReader.Get());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to setSourceReader in videocapture callback.", L"Error", MB_OK);
    return -1;
  }
  m_capWidth = m_videoCaptureCB->captureWidth();
  m_capHeight = m_videoCaptureCB->captureHeight();
  m_capFps = m_videoCaptureCB->captureFps();
  m_vcf = m_videoCaptureCB->captureFmt();

  // Create renderer.
  // And set the renderer.
  switch (m_vcf)
  {
    case VideoCaptureFormat::VideoCapFmt_NV12:
    {
      m_renderer = new DX11Nv12Renderer();
    }
    break;

    case VideoCaptureFormat::VideoCapFmt_YUY2:
    {
      m_renderer = new DX11YUY2Renderer();
    }
    break;

    case VideoCaptureFormat::VideoCapFmt_RGB32:
    {
      m_renderer = new DX11RGB32Renderer();
    }
    break;

    default:
    {
      MessageBoxW(nullptr, L"Not supported video capture format.", L"Error", MB_OK);
      return -1;
    }
  }
  m_renderer->init(previewWnd, m_capWidth, m_capHeight, m_capFps, m_vcf);
  hr = m_videoCaptureCB->setDx11Renerer(m_renderer);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to setDx11Renerer in videocapture callback.", L"Error", MB_OK);
    return -1;
  }

  // Ask for the first sample.
  hr = m_sourceReader->ReadSample(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , 0, nullptr, nullptr, nullptr, nullptr);
  if (FAILED(hr))
  {
    if (mediaSource)
    {
      mediaSource->Shutdown();
    }
    MessageBoxW(nullptr, L"Failed to read sample.", L"Error", MB_OK);
    return -1;
  }

  return 0;
}

VideoCaptureCallback* VideoCaptureManager::callbackFactory(VideoCaptureColorConvMode vcccm)
{
  switch (vcccm)
  {
    case VideoCaptureColorConvMode::DMO:
    {
      return new VideoCaptureCBDMO();
    }
    break;

    case VideoCaptureColorConvMode::Shader:
    {
      return new VideoCaptureCBShader();
    }
    break;
  }

  return new VideoCaptureCBDMO();
}