
#include "stdafx.h"
#include "dx11manager.h"
#include "captureenginesamplecallback.h"

using namespace Microsoft::WRL;

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

  ComPtr<IMFMediaBuffer> buf = nullptr;
  UINT32 pitch = 4 * 1280;
  hr = sample->ConvertToContiguousBuffer(&buf);
  if (FAILED(hr))
  {
    sample->Release();
    return E_FAIL;
  }

  byte* byteBuffer = nullptr;
  DWORD buffCurrLen = 0;
  hr = buf->Lock(&byteBuffer, NULL, &buffCurrLen);
  if (FAILED(hr))
  {
    buf->Unlock();
    sample->Release();
    return E_FAIL;
  }
  assert(buffCurrLen == (pitch * 720));

  bool result = manager::DX11Manager::getInstance().updateTexture(byteBuffer, buffCurrLen);
  if (!result)
  {
    sample->Release();
    buf->Unlock();
    return E_FAIL;
  }
  // I'm not sending events to MainWindow now, so I'm release here.
  sample->Release();
  buf->Unlock();

  return S_OK;
}
