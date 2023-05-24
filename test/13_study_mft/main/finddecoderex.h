
#ifndef FIND_DECODER_EX_h_
#define FIND_DECODER_EX_H_

#include "stdafx.h"

class FindDecoderEX
{
public:
  explicit FindDecoderEX() = default;
  ~FindDecoderEX() = default;

  HRESULT search(
    const GUID& subtype,        // Subtype
    BOOL bAudio,                // TRUE for audio, FALSE for video
    IMFTransform **ppDecoder    // Receives a pointer to the decoder.
    );
};

#endif // FIND_DECODER_EX_H_
