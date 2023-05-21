
#include "stdafx.h"
#include "dx11manager.h"
#include "timer.h"
#include "capturemanager.h"
#include "videocapturecallback.h"

using namespace Microsoft::WRL;

VideoCaptureCB::VideoCaptureCB()
  : m_ref(1)
  , m_sourceReader(nullptr)
{
  InitializeCriticalSection(&m_criticalSection);
}

VideoCaptureCB::~VideoCaptureCB()
{
  DeleteCriticalSection(&m_criticalSection);
}

STDMETHODIMP VideoCaptureCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(VideoCaptureCB, IMFSourceReaderCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) VideoCaptureCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_(ULONG) VideoCaptureCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

STDMETHODIMP VideoCaptureCB::OnReadSample(
    HRESULT hrStatus
    , DWORD dwStreamIndex
    , DWORD dwStreamFlags
    , LONGLONG llTimeStamp
    , IMFSample* sample)
{
  HRESULT hr = S_OK;

  EnterCriticalSection(&m_criticalSection);

  if (FAILED(hrStatus))
  {
    hr = hrStatus;
  }

  if (SUCCEEDED(hr))
  {
    if (sample)
    {
      DWORD dwTotalLength = 0;
      hr = sample->GetTotalLength(&dwTotalLength);
      if (SUCCEEDED(hr))
      {
        std::wcout << "Buffer size : " << dwTotalLength << std::endl;
      }

      sample->AddRef();

      ComPtr<IMFMediaBuffer> buf = nullptr;
      hr = sample->GetBufferByIndex(0, buf.GetAddressOf());
      if (FAILED(hr))
      {
        sample->Release();
        hr = E_FAIL;
      }

      byte* byteBuffer = nullptr;
      DWORD buffCurrLen = 0;
      hr = buf->Lock(&byteBuffer, NULL, &buffCurrLen);
      if (FAILED(hr))
      {
        buf->Unlock();
        sample->Release();
        hr = E_FAIL;
      }

      std::wcout << "Buffer current size : " << buffCurrLen << std::endl;

      sample->Release();
      buf->Unlock();
    }
  }

  // Request next frame
  hr = m_sourceReader->ReadSample(
    (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
    , 0
    , nullptr
    , nullptr
    , nullptr
    , nullptr
  );

  LeaveCriticalSection(&m_criticalSection);

#if 0
  Timer timer;

  ComPtr<IMFMediaBuffer> buf = nullptr;
  UINT32 pitch = 4 * 3840;
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
  assert(buffCurrLen == (pitch * 2160));

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

#endif
  return S_OK;
}
