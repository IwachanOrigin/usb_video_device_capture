
#ifndef CAPTURE_ENGINE_SAMPLE_CALLBACK_H_
#define CAPTURE_ENGINE_SAMPLE_CALLBACK_H_

#include "stdafx.h"

class CaptureEngineSampleCB : public IMFCaptureEngineOnSampleCallback
{
  long m_ref;
  uint32_t m_capWidth;
  uint32_t m_capHeight;

public:
  CaptureEngineSampleCB(const uint32_t& width, const uint32_t& height) : m_ref(1), m_capWidth(width), m_capHeight(height) {}

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  STDMETHODIMP OnSample(_In_ IMFSample* sample);
};

#endif // CAPTURE_ENGINE_SAMPLE_CALLBACK_H_
