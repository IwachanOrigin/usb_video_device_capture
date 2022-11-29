
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
  
}

