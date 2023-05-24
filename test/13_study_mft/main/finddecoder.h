
#ifndef FIND_DECODER_h_
#define FIND_DECODER_H_

#include "stdafx.h"

class FindDecoder
{
public:
  explicit FindDecoder() = default;
  ~FindDecoder() = default;

  HRESULT search(
    const GUID& subtype,        // Subtype
    BOOL bAudio,                // TRUE for audio, FALSE for video
    IMFTransform **ppDecoder    // Receives a pointer to the decoder.
    );
};

#endif // FIND_DECODER_H_
