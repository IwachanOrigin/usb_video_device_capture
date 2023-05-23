
#include "stdafx.h"
#include "utils.h"
#include <iostream>

using namespace Microsoft::WRL;

int main(int argc, char* argv[])
{
  HRESULT hr = S_OK;
  UINT32 count = 0;

  IMFActivate **ppActivate = NULL;         // Array of activation objects.
  ComPtr<IMFTransform> pDecoder = nullptr; // Pointer to the decoder.

  // Match WMV3 video.
  MFT_REGISTER_TYPE_INFO info = { MFMediaType_Video, MFVideoFormat_WMV3 };

  UINT32 unFlags = MFT_ENUM_FLAG_SYNCMFT
    | MFT_ENUM_FLAG_LOCALMFT
    | MFT_ENUM_FLAG_SORTANDFILTER;

  hr = MFTEnumEx(
    MFT_CATEGORY_VIDEO_DECODER
    , unFlags
    , &info // Input type
    , NULL // Output type
    , &ppActivate
    , &count);

  if (SUCCEEDED(hr) && count == 0)
  {
    hr = MF_E_TOPO_CODEC_NOT_FOUND;
  }

  std::wcout << "Decoder count : " << count << std::endl;

  // Create the first decoder in the list.
  if (SUCCEEDED(hr))
  {
    hr = ppActivate[0]->ActivateObject(IID_PPV_ARGS(pDecoder.GetAddressOf()));
  }

  for (UINT32 i = 0; i < count; i++)
  {
    ppActivate[i]->Release();
  }
  CoTaskMemFree(ppActivate);

  return 0;
}

