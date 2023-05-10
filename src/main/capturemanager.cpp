
#include "stdafx.h"
#include "utils.h"
#include "captureenginevideocallback.h"
#include "captureengineaudiocallback.h"
#include "capturemanager.h"

using namespace helper;
using namespace Microsoft::WRL;

// CaptureManagerCB
STDMETHODIMP  CaptureManager::CaptureEngineCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(CaptureEngineCB, IMFCaptureEngineOnEventCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) CaptureManager::CaptureEngineCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_ (ULONG) CaptureManager::CaptureEngineCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

STDMETHODIMP CaptureManager::CaptureEngineCB::OnEvent(_In_ IMFMediaEvent* event)
{
  GUID guidType;
  HRESULT hrStatus = S_OK;

  HRESULT hr = event->GetStatus(&hrStatus);
  if (FAILED(hr))
  {
    hrStatus = hr;
  }

  hr = event->GetExtendedType(&guidType);
  if (SUCCEEDED(hr))
  {
    if (m_isSleeping && m_manager != nullptr)
    {

      if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
      {
        m_manager->onPreviewStopped(hrStatus);
        SetEvent(m_manager->m_event);
      }
      else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
      {
        m_manager->onRecordStopped(hrStatus);
        SetEvent(m_manager->m_event);
      }
      else
      {
        SetEvent(m_manager->m_event);
      }
    }
    else
    {
      if (guidType == MF_CAPTURE_ENGINE_INITIALIZED)
      {
        if (m_manager != nullptr)
        {
          m_manager->onCapEngineInited(hrStatus);
          m_manager->setErrorID(hrStatus, 104);
          SetEvent(m_manager->m_event);
        }
      }
    }
    event->AddRef();
    // I'm not sending events to MainWindow now, so I'm release here.
    event->Release();
  }
  return S_OK;
}

// CaptureManagerCB

// CaptureManager

CaptureManager::CaptureManager()
  : m_captureEngine(nullptr)
  , m_capPrevSink(nullptr)
  , m_capCallback(nullptr)
  , m_isPreviewing(false)
  , m_isRecording(false)
  , m_isPhotoPending(false)
  , m_errorID(0)
  , m_event(nullptr)
  , m_pwrRequest(INVALID_HANDLE_VALUE)
  , m_isPowerRequestSet(false)
{
  WCHAR srs[] = L"CaptureEngine is recording!";
  REASON_CONTEXT powerContext = {};
  powerContext.Version = POWER_REQUEST_CONTEXT_VERSION;
  powerContext.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
  powerContext.Reason.SimpleReasonString = srs;

  m_pwrRequest = PowerCreateRequest(&powerContext);
}

CaptureManager::~CaptureManager()
{
  this->destroyCapEngine();
}

HRESULT CaptureManager::initCaptureManager(IUnknown* pVideoDevice, IUnknown* pAudioDevice)
{
  HRESULT hr = S_OK;
  ComPtr<IMFAttributes> attributes = nullptr;
  ComPtr<IMFCaptureEngineClassFactory> factory = nullptr;

  this->destroyCapEngine();

  m_event = CreateEvent(nullptr, false, false, nullptr);
  if (nullptr == m_event)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
  }

  m_capCallback = new (std::nothrow)CaptureEngineCB();
  if (nullptr == m_capCallback)
  {
    hr = E_OUTOFMEMORY;
    return hr;
  }

  m_capCallback->setCaptureManager(this);

  hr = MFCreateAttributes(attributes.GetAddressOf(), 1);
  if (FAILED(hr))
  {
    return hr;
  }

  // Create the factory object for the capture engine.
  hr = CoCreateInstance(CLSID_MFCaptureEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
  if (FAILED(hr))
  {
    return hr;
  }

  // Create and initialize the capture engine.
  hr = factory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(m_captureEngine.GetAddressOf()));
  if (FAILED(hr))
  {
    return hr;
  }

  hr = m_captureEngine->Initialize(m_capCallback.Get(), attributes.Get(), pAudioDevice, pVideoDevice);
  if (FAILED(hr))
  {
    return hr;
  }

  return hr;
}

// Handle an event from the capture engine.
HRESULT CaptureManager::onCapEvent(WPARAM wParam, LPARAM lParam)
{
  GUID guidType;
  HRESULT hrStatus = S_OK;

  IMFMediaEvent* mediaEvent = reinterpret_cast<IMFMediaEvent*>(wParam);
  HRESULT hr = mediaEvent->GetStatus(&hrStatus);
  if (FAILED(hr))
  {
    hrStatus = hr;
  }

  hr = mediaEvent->GetExtendedType(&guidType);
  if (SUCCEEDED(hr))
  {
    if (guidType == MF_CAPTURE_ENGINE_INITIALIZED)
    {
      this->onCapEngineInited(hrStatus);
    }
    else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STARTED)
    {
      this->onPreviewStarted(hrStatus);
    }
    else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
    {
      this->onPreviewStopped(hrStatus);
    }
    else if (guidType == MF_CAPTURE_ENGINE_RECORD_STARTED)
    {
      this->onRecordStarted(hrStatus);
    }
    else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
    {
      this->onRecordStopped(hrStatus);
    }
    else if (guidType == MF_CAPTURE_ENGINE_PHOTO_TAKEN)
    {
      m_isPhotoPending = false;
    }
    else if (guidType == MF_CAPTURE_ENGINE_ERROR)
    {
      this->destroyCapEngine();
    }
    else if (FAILED(hrStatus))
    {
      //
    }
  }

  mediaEvent->Release();
  SetEvent(m_event);
  return hrStatus;
}

void CaptureManager::onCapEngineInited(HRESULT& hr)
{
  if (hr == MF_E_NO_CAPTURE_DEVICES_AVAILABLE)
  {
    hr = S_OK; // No capture device. Not an application error.
  }
}

void CaptureManager::onPreviewStarted(HRESULT& hr)
{
  m_isPreviewing = SUCCEEDED(hr);
}

void CaptureManager::onPreviewStopped(HRESULT& hr)
{
  m_isPreviewing = false;
}

void CaptureManager::onRecordStarted(HRESULT& hr)
{
  m_isRecording = SUCCEEDED(hr);
}

void CaptureManager::onRecordStopped(HRESULT& hr)
{
  m_isRecording = false;
}

HRESULT CaptureManager::startPreview(const uint32_t& width, const uint32_t& height, const uint32_t& fpsNum)
{
  if (m_captureEngine == nullptr)
  {
    return MF_E_NOT_INITIALIZED;
  }

  if (m_isPreviewing)
  {
    return S_OK;
  }

  ComPtr<IMFCaptureSink> sink = nullptr;
  ComPtr<IMFMediaType> inputVideoMediaType = nullptr;
  ComPtr<IMFMediaType> outputVideoMediaType = nullptr;
  ComPtr<IMFMediaType> inputAudioMediaType = nullptr;
  ComPtr<IMFMediaType> outputAudioMediaType = nullptr;
  ComPtr<IMFCaptureSource> captureSource = nullptr;

  HRESULT hr = S_OK;

  // Get a pointer to the preview sink.
  if (m_capPrevSink == nullptr)
  {
    hr = m_captureEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PREVIEW, sink.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    hr = sink->QueryInterface(IID_PPV_ARGS(m_capPrevSink.GetAddressOf()));
    if (FAILED(hr))
    {
      return hr;
    }

    // Get a pointer to the capture source.
    hr = m_captureEngine->GetSource(captureSource.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    // Configure the video format for the preview sink.
    hr = captureSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, inputVideoMediaType.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    // format
    hr = utilCloneVideomediaType(inputVideoMediaType.Get(), MFVideoFormat_RGB32, outputVideoMediaType.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    // frame size
    hr = MFSetAttributeSize(outputVideoMediaType.Get(), MF_MT_FRAME_SIZE, width, height);
    if (FAILED(hr))
    {
      return hr;
    }

    // fps
    hr = MFSetAttributeRatio(outputVideoMediaType.Get(), MF_MT_FRAME_RATE, fpsNum, 1);
    if (FAILED(hr))
    {
      return hr;
    }

    // https://learn.microsoft.com/ja-jp/windows/win32/medfound/mf-mt-all-samples-independent-attribute
    hr = outputVideoMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr))
    {
      return hr;
    }

    // Configure the audio format for the preview sink.
    hr = captureSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, inputAudioMediaType.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    // audio format
    hr = utilCloneAudiomediaType(inputAudioMediaType.Get(), MFAudioFormat_PCM, outputAudioMediaType.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    // channel count
    hr = outputAudioMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
    if (FAILED(hr))
    {
      return hr;
    }

    // 
    hr = outputAudioMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000);
    if (FAILED(hr))
    {
      return hr;
    }

    // 
    hr = outputAudioMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4);
    if (FAILED(hr))
    {
      return hr;
    }

    // 
    hr = outputAudioMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 48000 * 4);
    if (FAILED(hr))
    {
      return hr;
    }

    // 
    hr = outputAudioMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    if (FAILED(hr))
    {
      return hr;
    }

    // 
    hr = outputAudioMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr))
    {
      return hr;
    }

    // Connect the video stream to the preview sink.
    DWORD dwSinkStreamVideoIndex = 0;
    hr = m_capPrevSink->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, outputVideoMediaType.Get(), nullptr, &dwSinkStreamVideoIndex);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = m_capPrevSink->SetSampleCallback(dwSinkStreamVideoIndex, new CaptureEngineVideoCB(width, height));
    if (FAILED(hr))
    {
      return hr;
    }

    // Connect the audio stream to the preview sink.
    DWORD dwSinkStreamAudioIndex = 0;
    hr = m_capPrevSink->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, outputAudioMediaType.Get(), nullptr, &dwSinkStreamAudioIndex);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = m_capPrevSink->SetSampleCallback(dwSinkStreamAudioIndex, new CaptureEngineAudioCB());
    if (FAILED(hr))
    {
      return hr;
    }
  }

  hr = m_captureEngine->StartPreview();
  if (!m_isPowerRequestSet && m_pwrRequest != INVALID_HANDLE_VALUE)
  {
    m_isPowerRequestSet = (TRUE == PowerSetRequest(m_pwrRequest, PowerRequestExecutionRequired));
  }

  return hr;
}

HRESULT CaptureManager::stopPreview()
{
  HRESULT hr = S_OK;
  if (m_captureEngine == nullptr)
  {
    return MF_E_NOT_INITIALIZED;
  }

  if (!m_isPreviewing)
  {
    return S_OK;
  }

  hr = m_captureEngine->StopPreview();
  if (FAILED(hr))
  {
    return hr;
  }
  this->waitForResult();

  if (m_isPowerRequestSet && m_pwrRequest != INVALID_HANDLE_VALUE)
  {
    PowerClearRequest(m_pwrRequest, PowerRequestExecutionRequired);
    m_isPowerRequestSet = false;
  }

  return hr;
}

HRESULT CaptureManager::startRecord(PCWSTR destinationFile)
{
  return S_OK;
}

HRESULT CaptureManager::stopRecord()
{
  return S_OK;
}

HRESULT CaptureManager::takePhoto(PCWSTR filename)
{
  return S_OK;
}

// CaptureManager
