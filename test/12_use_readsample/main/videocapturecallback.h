
#ifndef VIDEO_CAPTURE_CALLBACK_H_
#define VIDEO_CAPTURE_CALLBACK_H_

#include "stdafx.h"

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

  void setSourceReader(IMFSourceReader* sourceReader) { m_sourceReader = sourceReader; }

private:
  IMFSourceReader* m_sourceReader;
  CRITICAL_SECTION m_criticalSection;
};

#endif // VIDEO_CAPTURE_CALLBACK_H_
