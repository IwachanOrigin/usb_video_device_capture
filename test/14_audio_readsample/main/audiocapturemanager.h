
#ifndef AUDIO_CAPTURE_MANAGER_H_
#define AUDIO_CAPTURE_MANAGER_H_

#include "stdafx.h"
#include "audiocapturecallback.h"

using namespace Microsoft::WRL;

class AudioCaptureManager
{
public:
  static AudioCaptureManager& getInstance();

  int init(IMFActivate *pActivate);

private:
  explicit AudioCaptureManager();
  virtual ~AudioCaptureManager();
  explicit AudioCaptureManager(const AudioCaptureManager &);
  AudioCaptureManager &operator=(const AudioCaptureManager &);

  ComPtr<IMFSourceReader> m_sourceReader;
  wchar_t *m_wcSymbolicLink;
  ComPtr<AudioCaptureCB> m_audioCaptureCB;
};

#endif // AUDIO_CAPTURE_MANAGER_H_
