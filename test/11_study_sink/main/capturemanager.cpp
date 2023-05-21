
#include "stdafx.h"
#include "utils.h"
#include "captureenginesamplecallback.h"
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

HRESULT CaptureManager::initCaptureManager(IUnknown* pUnk)
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

HRESULT CaptureManager::startPreview()
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
  ComPtr<IMFMediaType> mediatype = nullptr;
  ComPtr<IMFMediaType> outputMediaType = nullptr;
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

    hr = sink->QueryInterface(IID_PPV_ARGS(&m_capPrevSink));
    if (FAILED(hr))
    {
      return hr;
    }

    // RendarHandle

    hr = m_captureEngine->GetSource(captureSource.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    // Configure the video format for the preview sink.
#if 0
    hr = captureSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, mediatype.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }
#else
    UINT32 index = getOptimizedFormatIndex(captureSource.Get());
    hr = captureSource->GetAvailableDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, (DWORD)index, mediatype.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    GUID subtype{ 0 };
    hr = mediatype->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (subtype != MFVideoFormat_MJPG)
    {
      hr = mediatype->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_MJPG);
      if (FAILED(hr))
      {
        return hr;
      }
    }

    // Found an output type.
    UINT32 rate = 0, den = 0, width = 0, height = 0;
    hr = MFGetAttributeSize(mediatype.Get(), MF_MT_FRAME_RATE, &rate, &den);
    rate /= den;
    hr = MFGetAttributeSize(mediatype.Get(), MF_MT_FRAME_SIZE, &width, &height);
    DbgPrint(L"width = %u, height=%u, rate=%u.\n", width, height, rate);

    hr = captureSource->SetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, mediatype.Get());
    if (FAILED(hr))
    {
      return hr;
    }
#endif

    // format
    hr = utilCloneVideomediaType(mediatype.Get(), MFVideoFormat_MJPG, outputMediaType.GetAddressOf());
    if (FAILED(hr))
    {
      return hr;
    }

    hr = outputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr))
    {
      return hr;
    }

    // Connect the video stream to the preview sink.
    DWORD dwSinkStreamIndex = 0;
    hr = m_capPrevSink->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW, mediatype.Get(), nullptr, &dwSinkStreamIndex);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = m_capPrevSink->SetSampleCallback(dwSinkStreamIndex, new CaptureEngineSampleCB());
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

UINT32 CaptureManager::getOptimizedFormatIndex(IMFCaptureSource *pSource)
{
  if (!pSource)
  {
    return 0;
  }

  UINT32 index = 0, wMax = 0, rMax = 0;
  for (DWORD i = 0; ; i++)
  {
    ComPtr<IMFMediaType> pType = nullptr;
    HRESULT hr = pSource->GetAvailableDeviceMediaType(
      (DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW,
      i,
      pType.GetAddressOf()
    );

    if (FAILED(hr)) { break; }

    if (SUCCEEDED(hr))
    {
      // Found an output type.
      GUID subtype{ 0 };
      hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
      //if (subtype == MFVideoFormat_YUY2)
      {
        UINT32 rate = 0, den = 0, width = 0, height = 0;
        hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_RATE, &rate, &den);
        rate /= den;
        hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_SIZE, &width, &height);
        DbgPrint(L"width=%u, height=%u, rate=%u\n", width, height, rate);
        if (width >= wMax && rate >= rMax)
        {
          wMax = width;
          rMax = rate;
          index = i;
        }
      }
    }
  }
  return index;
}

// CaptureManager
