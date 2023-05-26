
#ifndef CAPTURE_MANAGER_H_
#define CAPTURE_MANAGER_H_

#include "stdafx.h"
#include "videocapturecallback.h"

using namespace Microsoft::WRL;

class CaptureManager
{
public:
  static CaptureManager& getInstance();

  int init(IMFActivate *pActivate);

  uint32_t getCaptureWidth() { return m_capWidth; }
  uint32_t getCaptureHeight() { return m_capHeight; }
  uint32_t getCaptureFps() { return m_capFps; }

private:
  explicit CaptureManager();
  virtual ~CaptureManager();
  explicit CaptureManager(const CaptureManager &);
  CaptureManager &operator=(const CaptureManager &);

  ComPtr<IMFSourceReader> m_sourceReader;
  wchar_t *m_wcSymbolicLink;
  ComPtr<VideoCaptureCB> m_videoCaptureCB;
  uint32_t m_capWidth;
  uint32_t m_capHeight;
  uint32_t m_capFps;
};

#endif // CAPTURE_MANAGER_H_
