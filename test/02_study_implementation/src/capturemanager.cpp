
#include "capturemanager.h"

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
  if (m_isSleeping && m_manager != nullptr)
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
      if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
      {
        m_manager->onPreviewStopped(hrStatus);
        SetEvent(m_manager->m_event);
      }
    }
  }
}

