
#ifndef CAPTURE_ENGINE_SAMPLE_CALLBACK_H_
#define CAPTURE_ENGINE_SAMPLE_CALLBACK_H_

#include "stdafx.h"

class CaptureEngineSampleCB : public IMFCaptureEngineOnSampleCallback
{
  long m_ref;

public:
  CaptureEngineSampleCB() : m_ref(1) {}

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  STDMETHODIMP OnSample(_In_ IMFSample *sample);
};

#endif // CAPTURE_ENGINE_SAMPLE_CALLBACK_H_
