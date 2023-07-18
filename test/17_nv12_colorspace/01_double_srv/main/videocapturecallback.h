
#ifndef VIDEO_CAPTURE_CALLBACK_H_
#define VIDEO_CAPTURE_CALLBACK_H_

#include "stdafx.h"

using namespace Microsoft::WRL;

class VideoCaptureCB : public IMFSourceReaderCallback
{
  long m_ref;

public:
  explicit VideoCaptureCB();
  virtual ~VideoCaptureCB();

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IMFSourceReaderCallback methods
  STDMETHODIMP OnReadSample(
    HRESULT hrStatus
    , DWORD dwStreamIndex
    , DWORD dwStreamFlags
    , LONGLONG llTimeStamp
    , IMFSample* sample);
  STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) { return S_OK; };
  STDMETHODIMP OnFlush(DWORD) { return S_OK; }

  HRESULT setSourceReader(IMFSourceReader* sourceReader);
  uint32_t getCaptureWidth() { return m_capWidth; }
  uint32_t getCaptureHeight() { return m_capHeight; }
  uint32_t getCaptureFps() { return m_capFps; }

private:
  IMFSourceReader* m_sourceReader;
  ComPtr<IMFTransform> m_colorConvTransform;
  ComPtr<IMFMediaType> m_DecoderOutputMediaType;
  CRITICAL_SECTION m_criticalSection;
  uint32_t m_sampleCount;
  uint32_t m_capWidth;
  uint32_t m_capHeight;
  uint32_t m_capFps;

  uint32_t getOptimizedFormatIndex();
  bool isAcceptedFormat(const GUID& subtype);
  HRESULT setCaptureResolutionAndFps();
};

#endif // VIDEO_CAPTURE_CALLBACK_H_
