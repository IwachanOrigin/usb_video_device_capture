
#ifndef CAPTURE_MANAGER_H_
#define CAPTURE_MANAGER_H_

#include "stdafx.h"

using namespace Microsoft::WRL;

class CaptureManager
{
public:
  static CaptureManager& getInstance();

  int init(IMFActivate *pActivate);

private:
  explicit CaptureManager();
  virtual ~CaptureManager();
  explicit CaptureManager(const CaptureManager &);
  CaptureManager &operator=(const CaptureManager &);

  ComPtr<IMFSourceReader> m_sourceReader;
  wchar_t *m_wcSymbolicLink;
};

#endif // CAPTURE_MANAGER_H_
