
#include "stdafx.h"
#include "dxhelper.h"
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

  HRESULT hr = S_OK;
  LONGLONG   llVideoTimeStamp = 0, llSampleDuration = 0;
  int        sampleCount = 0;
  DWORD      sampleFlags = 0;

  hr = sample->SetSampleTime(llVideoTimeStamp);
  assert(hr == S_OK);

  hr = sample->GetSampleDuration(&llSampleDuration);
  assert(hr == S_OK);

  hr = sample->GetSampleFlags(&sampleFlags);
  assert(hr == S_OK);


  IMFMediaBuffer* buf = nullptr;
  DWORD bufLength;
  hr = sample->CopyToBuffer(buf);
  hr = buf->GetCurrentLength(&bufLength);

  byte* byteBuffer = nullptr;
  DWORD buffMaxLen = 0, buffCurrLen = 0;
  hr = buf->Lock(&byteBuffer, &buffMaxLen, &buffCurrLen);
  assert(hr == S_OK);

  if (!m_targetTexture)
  {
    m_targetTexture = new Render::Texture();
    int texture_height = m_height * 2;
    if (!m_targetTexture->create(m_width, texture_height, DXGI_FORMAT_R8_UNORM, true))
    {
      return S_FALSE;
    }
  }

  DWORD dwTotalLength = 0;
  hr = sample->GetTotalLength(&dwTotalLength);
  if (SUCCEEDED(hr))
  {
    std::wcout << "Buffer size : " << dwTotalLength << std::endl;
  }

  buf->Unlock();
  SAFE_RELEASE(buf);
  sampleCount++;

  sample->AddRef();
  // I'm not sending events to MainWindow now, so I'm release here.
  SAFE_RELEASE(sample);

  return S_OK;
}
