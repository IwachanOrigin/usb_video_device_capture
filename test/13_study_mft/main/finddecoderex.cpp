
#include "finddecoderex.h"

HRESULT FindDecoderEx::search(
    const GUID& subtype,        // Subtype
    BOOL bAudio,                // TRUE for audio, FALSE for video
    IMFTransform **ppDecoder    // Receives a pointer to the decoder.
    )
{
  HRESULT hr = S_OK;
  UINT32 count = 0;

  IMFActivate **ppActivate = NULL;         // Array of activation objects.
  ComPtr<IMFTransform> pDecoder = nullptr; // Pointer to the decoder.

  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    std::cerr << "Failed to MFStartup." << std::endl;
    return -1;
  }

  // Match WMV3 video.
  MFT_REGISTER_TYPE_INFO info = { MFMediaType_Video, MFVideoFormat_H264 };

  UINT32 unFlags = MFT_ENUM_FLAG_HARDWARE |
    MFT_ENUM_FLAG_ASYNCMFT |
                 MFT_ENUM_FLAG_SYNCMFT  |
                 MFT_ENUM_FLAG_LOCALMFT |
                 MFT_ENUM_FLAG_SORTANDFILTER;

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

  ComPtr<IMFTransform> transform = nullptr;
  ComPtr<IMFAttributes> attributes = nullptr;

  hr = pDecoder->GetAttributes(attributes.GetAddressOf());
  if (FAILED(hr))
  {
    std::wcerr << "Failed to get attribute form decoder." << std::endl;
    return -1;
  }

  UINT32 nameLenght = 0;
  std::wstring name = L"";

  hr = attributes->GetStringLength(MFT_FRIENDLY_NAME_Attribute, &nameLenght);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to get name length form attributes." << std::endl;
    return -1;
  }

  name.resize(nameLenght + 1);
  hr = attributes->GetString(MFT_FRIENDLY_NAME_Attribute, &name[0], (UINT32)name.size(), &nameLenght);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to get name form attributes." << std::endl;
    return -1;
  }

  name.resize(nameLenght);

  std::wcout << "Decoder name = " << name << std::endl;
}
