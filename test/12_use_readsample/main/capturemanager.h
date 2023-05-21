
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
  int nextFrame();

private:
  explicit CaptureManager();
  virtual ~CaptureManager();
  explicit CaptureManager(const CaptureManager &);
  CaptureManager &operator=(const CaptureManager &);

  ComPtr<IMFSourceReader> m_sourceReader;
  wchar_t *m_wcSymbolicLink;
  ComPtr<VideoCaptureCB> m_videoCaptureCB;
};

#endif // CAPTURE_MANAGER_H_
