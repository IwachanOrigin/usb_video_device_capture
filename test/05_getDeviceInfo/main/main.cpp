
#include <iostream>
#include <string>

#include <mfapi.h>
#include <mfidl.h>

#include "devicesinfo.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mf.lib")

int main(int argc, char* argv[])
{
  // Initialize
  HRESULT hr;
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    return hr;
  }

  DevicesInfo devices;
  devices.writeDeviceNameList();

  // Release
  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  return 0;
}
