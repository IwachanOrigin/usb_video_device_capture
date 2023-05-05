
#ifndef CAPTURE_ENGINE_AUDIO_CALLBACK_H_
#define CAPTURE_ENGINE_AUDIO_CALLBACK_H_

#include "stdafx.h"

class CaptureEngineAudioCB : public IMFCaptureEngineOnSampleCallback
{
  long m_ref;

public:
  CaptureEngineAudioCB() : m_ref(1) {}

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  STDMETHODIMP OnSample(_In_ IMFSample* sample);
};

#endif // CAPTURE_ENGINE_AUDIO_CALLBACK_H_
