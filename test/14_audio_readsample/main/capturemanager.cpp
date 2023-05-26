
#include "capturemanager.h"
#include "audiocapturecallback.h"

CaptureManager::CaptureManager()
  : m_sourceReader(nullptr)
  , m_wcSymbolicLink(nullptr)
  , m_audioCaptureCB(new AudioCaptureCB())
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
    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK
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
    , m_audioCaptureCB.Get()
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

  hr = m_audioCaptureCB->setSourceReader(m_sourceReader.Get());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to setSourceReader in videocapture callback.", L"Error", MB_OK);
    return -1;
  }

  // TODO: MediaType setting...MJPG, YUY2

  // Ask for the first sample.
  hr = m_sourceReader->ReadSample(
    (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM
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
