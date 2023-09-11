
#ifndef VIDEO_CAPTURE_CALLBACK_H_
#define VIDEO_CAPTURE_CALLBACK_H_

#include "stdafx.h"
#include "videocaptureformat.h"
#include "dx11baserenderer.h"

using namespace renderer;

class VideoCaptureCallback : public IMFSourceReaderCallback
{
public:
  explicit VideoCaptureCallback() = default;
  virtual ~VideoCaptureCallback() = default;

  // IUnknown
  virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppv) = 0;
  virtual STDMETHODIMP_(ULONG) AddRef() = 0;
  virtual STDMETHODIMP_(ULONG) Release() = 0;

  // IMFSourceReaderCallback methods
  virtual STDMETHODIMP OnReadSample(
    HRESULT hrStatus
    , DWORD dwStreamIndex
    , DWORD dwStreamFlags
    , LONGLONG llTimeStamp
    , IMFSample* sample) = 0;
  virtual STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) = 0;
  virtual STDMETHODIMP OnFlush(DWORD) = 0;

  virtual HRESULT setSourceReader(IMFSourceReader* sourceReader) = 0;
  virtual HRESULT setDx11Renerer(DX11BaseRenderer* renderer) = 0;
  virtual uint32_t captureWidth() const = 0;
  virtual uint32_t captureHeight() const = 0;
  virtual uint32_t captureFps() const = 0;
  virtual VideoCaptureFormat captureFmt() const = 0;
};

#endif // VIDEO_CAPTURE_CALLBACK_H_
