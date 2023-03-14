
#ifndef CAPTURE_ENGINE_SAMPLE_CALLBACK_H_
#define CAPTURE_ENGINE_SAMPLE_CALLBACK_H_

#include "stdafx.h"
#include "texture.h"

class CaptureEngineSampleCB : public IMFCaptureEngineOnSampleCallback
{
  long m_ref;

public:
  CaptureEngineSampleCB() : m_ref(1), m_targetTexture(nullptr), m_width(0), m_height(0) {}

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  STDMETHODIMP OnSample(_In_ IMFSample* sample);

  //
  void setWidth(const uint32_t width) { m_width = width; }
  void setHeight(const uint32_t height) { m_height = height; }
  Render::Texture* getTexture() { return m_targetTexture; }

private:
  Render::Texture* m_targetTexture;
  uint32_t m_width;
  uint32_t m_height;

};

#endif // CAPTURE_ENGINE_SAMPLE_CALLBACK_H_
