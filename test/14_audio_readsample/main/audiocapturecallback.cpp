
#include "stdafx.h"
#include "timer.h"
#include "utils.h"
#include "capturemanager.h"
#include "audiocapturecallback.h"

using namespace Microsoft::WRL;
using namespace helper;

AudioCaptureCB::AudioCaptureCB()
  : m_ref(1)
  , m_sourceReader(nullptr)
  , m_DecoderOutputMediaType(nullptr)
  , m_sampleCount(0)
{
  InitializeCriticalSection(&m_criticalSection);
}

AudioCaptureCB::~AudioCaptureCB()
{
  DeleteCriticalSection(&m_criticalSection);
}

STDMETHODIMP AudioCaptureCB::QueryInterface(REFIID riid, void** ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(AudioCaptureCB, IMFSourceReaderCallback)
    , { 0 }
  };
  return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) AudioCaptureCB::AddRef()
{
  return InterlockedIncrement(&m_ref);
}

STDMETHODIMP_(ULONG) AudioCaptureCB::Release()
{
  LONG ref = InterlockedDecrement(&m_ref);
  if (ref == 0)
  {
    delete this;
  }
  return ref;
}

HRESULT AudioCaptureCB::setSourceReader(IMFSourceReader* sourceReader)
{
  HRESULT hr = E_FAIL;

  if (!sourceReader)
  {
    return hr;
  }
  m_sourceReader = sourceReader;

  hr = MFCreateMediaType(m_DecoderOutputMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create media type.", L"Error", MB_OK);
    return hr;
  }

  ComPtr<IMFMediaType> deviceCurrentMediaType = nullptr;
  hr = m_sourceReader->GetCurrentMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM
    , deviceCurrentMediaType.GetAddressOf()
    );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get the camera current media type.", L"Error", MB_OK);
    return hr;
  }

  hr = utilCloneAudiomediaType(deviceCurrentMediaType.Get(), MFAudioFormat_PCM, m_DecoderOutputMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to copy the camera current media type to the decoder output media type.", L"Error", MB_OK);
    return hr;
  }

  // channel count
  hr = m_DecoderOutputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
  if (FAILED(hr))
  {
    return hr;
  }

  // 
  hr = m_DecoderOutputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000);
  if (FAILED(hr))
  {
    return hr;
  }

  // 
  hr = m_DecoderOutputMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4);
  if (FAILED(hr))
  {
    return hr;
  }

  // 
  hr = m_DecoderOutputMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 48000 * 4);
  if (FAILED(hr))
  {
    return hr;
  }

  // 
  hr = m_DecoderOutputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
  if (FAILED(hr))
  {
    return hr;
  }

  // 
  hr = m_DecoderOutputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = m_sourceReader->SetCurrentMediaType(
    (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM
    , NULL
    , m_DecoderOutputMediaType.Get()
  );
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set the camera media type.", L"Error", MB_OK);
    return hr;
  }

  return hr;
}

STDMETHODIMP AudioCaptureCB::OnReadSample(
    HRESULT hrStatus
    , DWORD dwStreamIndex
    , DWORD dwStreamFlags
    , LONGLONG llTimeStamp
    , IMFSample* sample)
{
  HRESULT hr = S_OK;

  EnterCriticalSection(&m_criticalSection);

  if (SUCCEEDED(hrStatus))
  {
    if (sample)
    {
      sample->AddRef();

      std::wcout << "Processing sample : " << m_sampleCount << std::endl;
      LONGLONG llSampleDuration = 0;
      DWORD sampleFlags = 0;
      hr = sample->SetSampleTime(llTimeStamp);
      hr = sample->GetSampleDuration(&llSampleDuration);
      hr = sample->GetSampleFlags(&sampleFlags);
      printf("Sample flags %d, sample duration %I64d, sample time %I64d\n", sampleFlags, llSampleDuration, llTimeStamp);

      ComPtr<IMFMediaBuffer> buf = nullptr;
      hr = sample->ConvertToContiguousBuffer(buf.GetAddressOf());
      if (FAILED(hr))
      {
        std::cerr << "Failed the ConvertToContiguousBuffer." << std::endl;
      }

      byte* byteBuffer = nullptr;
      DWORD buffCurrLen = 0;
      hr = buf->Lock(&byteBuffer, NULL, &buffCurrLen);
      if (FAILED(hr))
      {
        std::cerr << "Failed the ConvertToContiguousBuffer." << std::endl;
      }
      std::wcout << "current size : " << buffCurrLen << std::endl;
      buf->Unlock();

      sample->Release();
      m_sampleCount++;
    }
  }

  // Request next frame
  hr = m_sourceReader->ReadSample(
    (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM
    , 0
    , nullptr
    , nullptr
    , nullptr
    , nullptr
    );

  LeaveCriticalSection(&m_criticalSection);

  return S_OK;
}
