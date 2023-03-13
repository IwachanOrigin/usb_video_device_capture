
#include "stdafx.h"
#include "captureenginesamplecallback.h"

STDMETHODIMP CaptureEngineSampleCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(CaptureEngineSampleCB, IMFCaptureEngineOnSampleCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CaptureEngineSampleCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_(ULONG) CaptureEngineSampleCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

STDMETHODIMP CaptureEngineSampleCB::OnSample(_In_ IMFSample* sample)
{
  if (sample == nullptr)
  {
    return S_OK;
  }

  DWORD dwTotalLength = 0;
  HRESULT hr = sample->GetTotalLength(&dwTotalLength);
  if (SUCCEEDED(hr))
  {
    std::wcout << "Buffer size : " << dwTotalLength << std::endl;
  }

  sample->AddRef();
  // I'm not sending events to MainWindow now, so I'm release here.
  sample->Release();

  return S_OK;
}
