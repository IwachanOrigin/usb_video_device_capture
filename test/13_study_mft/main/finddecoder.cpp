
#include "utils.h"
#include "finddecoder.h"

HRESULT FindDecoder::search(
    const GUID& subtype,        // Subtype
    BOOL bAudio,                // TRUE for audio, FALSE for video
    IMFTransform **ppDecoder    // Receives a pointer to the decoder.
    )
{
  HRESULT hr = S_OK;
  UINT32 count = 0;

  CLSID *ppCLSIDs = nullptr;

  MFT_REGISTER_TYPE_INFO info = { 0 };

  info.guidMajorType = bAudio ? MFMediaType_Audio : MFMediaType_Video;
  info.guidSubtype = subtype;

  hr = MFTEnum(
    bAudio ? MFT_CATEGORY_AUDIO_DECODER : MFT_CATEGORY_VIDEO_DECODER,
    0,      // Reserved
    &info,  // Input type
    NULL,   // Output type
    NULL,   // Reserved
    &ppCLSIDs,
    &count
    );

  if (SUCCEEDED(hr) && count == 0)
  {
    hr = MF_E_TOPO_CODEC_NOT_FOUND;
  }

  // Create the first decoder in the list.

  if (SUCCEEDED(hr))
  {
    hr = CoCreateInstance(ppCLSIDs[0], NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDecoder));
    LPWSTR pszName;
    MFT_REGISTER_TYPE_INFO** inputTypes = nullptr;
    MFT_REGISTER_TYPE_INFO** outputTypes = nullptr;
    UINT32 cInputTypes = 0;
    UINT32 cOutputTypes = 0;

    hr = MFTGetInfo(ppCLSIDs[0], &pszName, inputTypes, &cInputTypes, outputTypes, &cOutputTypes, NULL);

    if (SUCCEEDED(hr))
    {
        wprintf(L"MFT Name: %s\n", pszName);
        wprintf(L"Number of input types: %u\n", cInputTypes);
        wprintf(L"Number of output types: %u\n", cOutputTypes);
        for (UINT32 i = 0; i < cInputTypes; i++)
        {
          std::string name = helper::getGUIDNameConst(inputTypes[i]->guidSubtype);
          std::wstring wsName = std::wstring(name.begin(), name.end());
          wprintf(L"input subtype name: %s\n", wsName.c_str());
        }
        for (UINT32 i = 0; i < cOutputTypes; i++)
        {
          std::string name = helper::getGUIDNameConst(outputTypes[i]->guidSubtype);
          std::wstring wsName = std::wstring(name.begin(), name.end());
          wprintf(L"output subtype name: %s\n", wsName.c_str());
        }
    }
    else
    {
        wprintf(L"Error retrieving MFT info: %x\n", hr);
    }

    CoTaskMemFree(pszName);
    CoTaskMemFree(inputTypes);
    CoTaskMemFree(outputTypes);
  }

  CoTaskMemFree(ppCLSIDs);
  return hr;
}
