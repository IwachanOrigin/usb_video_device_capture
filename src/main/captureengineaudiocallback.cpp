
#include "stdafx.h"
#include "captureengineaudiocallback.h"

using namespace Microsoft::WRL;

STDMETHODIMP CaptureEngineAudioCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(CaptureEngineAudioCB, IMFCaptureEngineOnSampleCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CaptureEngineAudioCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_(ULONG) CaptureEngineAudioCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

STDMETHODIMP CaptureEngineAudioCB::OnSample(_In_ IMFSample* sample)
{
  if (sample == nullptr)
  {
    return S_OK;
  }

  sample->Release();

  return S_OK;
}
