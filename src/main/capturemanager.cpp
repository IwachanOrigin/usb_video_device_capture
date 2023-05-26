
#include "capturemanager.h"
#include "videocapturecallback.h"

CaptureManager::CaptureManager()
  : m_sourceReader(nullptr)
  , m_wcSymbolicLink(nullptr)
  , m_videoCaptureCB(new VideoCaptureCB())
  , m_capWidth(0)
  , m_capHeight(0)
  , m_capFps(0)
{
}

CaptureManager::~CaptureManager()
{
}

CaptureManager& CaptureManager::getInstance()
{
  static CaptureManager inst;
  return inst;
}

int CaptureManager::init(IMFActivate *pActivate)
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
  m_capWidth = m_videoCaptureCB->getCaptureWidth();
  m_capHeight = m_videoCaptureCB->getCaptureHeight();
  m_capFps = m_videoCaptureCB->getCaptureFps();

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
