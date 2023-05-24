
#include "stdafx.h"
#include "utils.h"
#include <iostream>
#include "finddecoder.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "wmcodecdspuuid.lib")

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

using namespace Microsoft::WRL;

int main(int argc, char* argv[])
{
  HRESULT hr = S_OK;

  ComPtr<IMFTransform> pDecoder = nullptr; // Pointer to the decoder.

  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    std::cerr << "Failed to MFStartup." << std::endl;
    return -1;
  }

  FindDecoder fd;
  hr = fd.search(MFVideoFormat_MJPG, FALSE, pDecoder.GetAddressOf());
  if (FAILED(hr))
  {
    std::cerr << "Failed to search." << std::endl;
    return -1;
  }

  return 0;
}

