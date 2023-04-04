
#include <iostream>
#include <string>

#include <mfapi.h>
#include <mfidl.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

int main(int argc, char* argv[])
{
  // Initialize
  HRESULT hr;
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    return hr;
  }


  // Release
  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  return 0;
}
