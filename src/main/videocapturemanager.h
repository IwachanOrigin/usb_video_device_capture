
#ifndef VIDEO_CAPTURE_MANAGER_H_
#define VIDEO_CAPTURE_MANAGER_H_

#include "stdafx.h"
#include "videocapturecallback.h"

using namespace Microsoft::WRL;

class VideoCaptureManager
{
public:
  static VideoCaptureManager& getInstance();

  int init(IMFActivate *pActivate);

  uint32_t getCaptureWidth() { return m_capWidth; }
  uint32_t getCaptureHeight() { return m_capHeight; }
  uint32_t getCaptureFps() { return m_capFps; }

private:
  explicit VideoCaptureManager();
  virtual ~VideoCaptureManager();
  explicit VideoCaptureManager(const VideoCaptureManager &);
  VideoCaptureManager &operator=(const VideoCaptureManager &);

  ComPtr<IMFSourceReader> m_sourceReader;
  wchar_t *m_wcSymbolicLink;
  ComPtr<VideoCaptureCB> m_videoCaptureCB;
  uint32_t m_capWidth;
  uint32_t m_capHeight;
  uint32_t m_capFps;
};

#endif // VIDEO_CAPTURE_MANAGER_H_
