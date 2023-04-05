
#include <iostream>
#include <string>

#include <mfapi.h>
#include <mfidl.h>

#include "devicesinfo.h"

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

  DevicesInfo devices;
  devices.writeDeviceNameList();

  std::wcout << "Please Input Your USB Camera Device Index Number : ";

  std::string input = "";
  std::getline(std::cin, input);
  int devindex = std::stoi(input);

  devices.setCurrentVideoDeviceIndex(devindex);
  //devices.setCurrentAudioDeviceIndex(devindex);
  devices.writeDeviceMediaInfoList();


  // Release
  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  return 0;
}
