
#include "stdafx.h"
#include "utils.h"
#include "captureenginesamplecallback.h"
#include "capture_texture.h"

using namespace dx_engine;

// CaptureManagerCB
STDMETHODIMP  CaptureTexture::CaptureEngineCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(CaptureEngineCB, IMFCaptureEngineOnEventCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) CaptureTexture::CaptureEngineCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_ (ULONG) CaptureTexture::CaptureEngineCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

STDMETHODIMP CaptureTexture::CaptureEngineCB::OnEvent(_In_ IMFMediaEvent* event)
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

HRESULT CaptureTexture::initCaptureTexture(IUnknown* pUnk)
{
  HRESULT hr = S_OK;
  IMFAttributes* attributes = nullptr;
  IMFCaptureEngineClassFactory* factory = nullptr;

  this->destroyCapEngine();

  m_event = CreateEvent(nullptr, false, false, nullptr);
  if (nullptr == m_event)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    goto Exit;
  }

  m_capCallback = new (std::nothrow)CaptureEngineCB();
  if (nullptr == m_capCallback)
  {
    hr = E_OUTOFMEMORY;
    goto Exit;
  }

  m_capCallback->setCaptureManager(this);

  hr = MFCreateAttributes(&attributes, 1);
  if (FAILED(hr))
  {
    goto Exit;
  }

  // DXGI

  // Create the factory object for the capture engine.
  hr = CoCreateInstance(CLSID_MFCaptureEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
  if (FAILED(hr))
  {
    goto Exit;
  }

  // Create and initialize the capture engine.
  hr = factory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(&m_captureEngine));
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = m_captureEngine->Initialize(m_capCallback, attributes, nullptr, pUnk);
  if (FAILED(hr))
  {
    goto Exit;
  }

Exit:
  if (nullptr != attributes)
  {
    attributes->Release();
    attributes = nullptr;
  }

  if (nullptr != factory)
  {
    factory->Release();
    factory = nullptr;
  }

  return hr;
}

// Handle an event from the capture engine.
HRESULT CaptureTexture::onCapEvent(WPARAM wParam, LPARAM lParam)
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
    // DEBUG CODE

    //
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

void CaptureTexture::onCapEngineInited(HRESULT& hr)
{
  if (hr == MF_E_NO_CAPTURE_DEVICES_AVAILABLE)
  {
    hr = S_OK; // No capture device. Not an application error.
  }
}

void CaptureTexture::onPreviewStarted(HRESULT& hr)
{
  m_isPreviewing = SUCCEEDED(hr);
}

void CaptureTexture::onPreviewStopped(HRESULT& hr)
{
  m_isPreviewing = false;
}

void CaptureTexture::onRecordStarted(HRESULT& hr)
{
  m_isRecording = SUCCEEDED(hr);
}

void CaptureTexture::onRecordStopped(HRESULT& hr)
{
  m_isRecording = false;
}

HRESULT CaptureTexture::startPreview()
{
  if (m_captureEngine == nullptr)
  {
    return MF_E_NOT_INITIALIZED;
  }

  if (m_isPreviewing)
  {
    return S_OK;
  }

  IMFCaptureSink* sink = nullptr;
  IMFMediaType* mediatype = nullptr;
  IMFMediaType* mediatype2 = nullptr;
  IMFCaptureSource* captureSource = nullptr;

  HRESULT hr = S_OK;

  // Get a pointer to the preview sink.
  if (m_capPrevSink == nullptr)
  {
    hr = m_captureEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PREVIEW, &sink);
    if (FAILED(hr))
    {
      goto Exit;
    }

    hr = sink->QueryInterface(IID_PPV_ARGS(&m_capPrevSink));
    if (FAILED(hr))
    {
      goto Exit;
    }

    // RendarHandle

    hr = m_captureEngine->GetSource(&captureSource);
    if (FAILED(hr))
    {
      goto Exit;
    }

    // Configure the video format for the preview sink.
    hr = captureSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, &mediatype);
    if (FAILED(hr))
    {
      goto Exit;
    }

    hr = utilCloneVideomediaType(mediatype, MFVideoFormat_RGB32, &mediatype2);
    if (FAILED(hr))
    {
      goto Exit;
    }

    hr = mediatype2->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr))
    {
      goto Exit;
    }

    // Connect the video stream to the preview sink.
    DWORD dwSinkStreamIndex = 0;
    hr = m_capPrevSink->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, mediatype2, nullptr, &dwSinkStreamIndex);
    if (FAILED(hr))
    {
      goto Exit;
    }

    hr = m_capPrevSink->SetSampleCallback(dwSinkStreamIndex, new CaptureEngineSampleCB());
    if (FAILED(hr))
    {
      goto Exit;
    }
  }

  hr = m_captureEngine->StartPreview();
  if (!m_isPowerRequestSet && m_pwrRequest != INVALID_HANDLE_VALUE)
  {
    m_isPowerRequestSet = (TRUE == PowerSetRequest(m_pwrRequest, PowerRequestExecutionRequired));
  }

Exit:
  SAFE_RELEASE(sink);
  SAFE_RELEASE(mediatype);
  SAFE_RELEASE(mediatype2);
  SAFE_RELEASE(captureSource);

  return hr;
}

HRESULT CaptureTexture::stopPreview()
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

HRESULT CaptureTexture::startRecord(PCWSTR destinationFile)
{
  return S_OK;
}

HRESULT CaptureTexture::stopRecord()
{
  return S_OK;
}

HRESULT CaptureTexture::takePhoto(PCWSTR filename)
{
  return S_OK;
}

// CaptureManager
