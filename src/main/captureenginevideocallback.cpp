
#include "stdafx.h"
#include "dx11manager.h"
#include "timer.h"
#include "captureenginevideocallback.h"

using namespace Microsoft::WRL;

STDMETHODIMP CaptureEngineVideoCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(CaptureEngineVideoCB, IMFCaptureEngineOnSampleCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CaptureEngineVideoCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_(ULONG) CaptureEngineVideoCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

STDMETHODIMP CaptureEngineVideoCB::OnSample(_In_ IMFSample* sample)
{
  if (sample == nullptr)
  {
    return S_OK;
  }

  Timer timer;

  DWORD dwTotalLength = 0;
  HRESULT hr = sample->GetTotalLength(&dwTotalLength);
  if (SUCCEEDED(hr))
  {
    //std::wcout << "Buffer size : " << dwTotalLength << std::endl;
  }

  sample->AddRef();

  // RGB32 is 4 bit per pixel
  UINT32 pitch = 4 * m_capWidth;
  ComPtr<IMFMediaBuffer> buf = nullptr;
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
  assert(buffCurrLen == (pitch * m_capHeight));

  bool result = manager::DX11Manager::getInstance().updateTexture(byteBuffer, buffCurrLen);
  if (!result)
  {
    sample->Release();
    buf->Unlock();
    MessageBoxW(nullptr, L"Failed to update texture.", L"Error", MB_OK);
    return E_FAIL;
  }

  // Rendering
  result = manager::DX11Manager::getInstance().render();
  if (!result)
  {
    sample->Release();
    buf->Unlock();
    MessageBoxW(nullptr, L"Failed to rendering.", L"Error", MB_OK);
    return E_FAIL;
  }

  sample->Release();
  buf->Unlock();

  std::wcout << "Timer : " << timer.elapsed() << std::endl;

  return S_OK;
}
