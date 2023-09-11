
#ifndef VIDEO_CAPTURE_CALLBACK_DMO_H_
#define VIDEO_CAPTURE_CALLBACK_DMO_H_

#include "stdafx.h"
#include "dx11baserenderer.h"
#include "videocaptureformat.h"
#include "videocapturecallback.h"

using namespace Microsoft::WRL;
using namespace renderer;

class VideoCaptureCBDMO : public VideoCaptureCallback
{
  long m_ref;

public:
  explicit VideoCaptureCBDMO();
  virtual ~VideoCaptureCBDMO();

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
  HRESULT setDx11Renerer(DX11BaseRenderer* renderer);
  uint32_t captureWidth() const { return m_capWidth; }
  uint32_t captureHeight() const { return m_capHeight; }
  uint32_t captureFps() const { return m_capFps; }
  VideoCaptureFormat captureFmt() const { return m_vcf; }

private:
  IMFSourceReader* m_sourceReader;
  DX11BaseRenderer* m_renderer;
  ComPtr<IMFTransform> m_colorConvTransform;
  ComPtr<IMFMediaType> m_DecoderOutputMediaType;
  CRITICAL_SECTION m_criticalSection;
  uint32_t m_sampleCount;
  uint32_t m_capWidth;
  uint32_t m_capHeight;
  uint32_t m_capFps;
  VideoCaptureFormat m_vcf;

  uint32_t getOptimizedFormatIndex();
  bool isAcceptedFormat(const GUID& subtype, VideoCaptureFormat& fmt);
  HRESULT setCaptureResolutionAndFps();
};

#endif // VIDEO_CAPTURE_CALLBACK_DMO_H_
