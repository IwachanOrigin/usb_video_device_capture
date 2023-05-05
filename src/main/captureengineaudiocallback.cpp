
#include "stdafx.h"
#include "audiodevicemanager.h"
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

  DWORD dwTotalLength = 0;
  HRESULT hr = sample->GetTotalLength(&dwTotalLength);
  if (SUCCEEDED(hr))
  {
    //std::wcout << "Buffer size : " << dwTotalLength << std::endl;
  }

  sample->AddRef();

  ComPtr<IMFMediaBuffer> buf = nullptr;
  hr = sample->ConvertToContiguousBuffer(buf.GetAddressOf());
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
#if 0
  // Audio rendering
  if (AudioDeviceManager::getInstance().getStatus())
  {
    bool result = AudioDeviceManager::getInstance().render(byteBuffer, buffCurrLen);
    if (!result)
    {
      std::wcout << "Failed to render audio." << std::endl;
    }
  }
#endif

  sample->Release();
  buf->Unlock();

  return S_OK;
}
