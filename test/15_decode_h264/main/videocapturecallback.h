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

private:
  IMFSourceReader* m_sourceReader;
  ComPtr<IMFTransform> m_h264ToNv12Transform;
  ComPtr<IMFTransform> m_colorConvTransform;
  ComPtr<IMFMediaType> m_h264ToNv12MediaType;
  ComPtr<IMFMediaType> m_DecoderOutputMediaType;
  CRITICAL_SECTION m_criticalSection;
  UINT32 m_sampleCount;

  UINT32 getOptimizedFormatIndex();
  void outputError(const std::wstring& errMsg, IMFSample* sample);
};

#endif // VIDEO_CAPTURE_CALLBACK_H_
