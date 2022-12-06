
#include "stdafx.h"
#include "dxhelper.h"
#include "win32messagehandler.h"
#include "capturemanager.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "powrprof")

using namespace helper;
using namespace message_handler;

void getVideoDevices(IMFActivate*** pppRawDevice, uint32_t& count)
{
  std::shared_ptr<IMFAttributes> pAttributes;
  IMFAttributes* pRawAttributes = nullptr;
  ThrowIfFailed(MFCreateAttributes(&pRawAttributes, 1));
  pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

  ThrowIfFailed(pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));

  count = 0;
  IMFActivate** ppRawDevice = nullptr;
  ThrowIfFailed(MFEnumDeviceSources(pAttributes.get(), &ppRawDevice, &count));

  for (uint32_t i = 0; i < count; i++)
  {
    wchar_t* buffer = nullptr;
    uint32_t length = 0;
    ThrowIfFailed(ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length));
    std::wcout << "No. " << i << " : " << buffer << std::endl;
  }
  pppRawDevice = &ppRawDevice;
}

int main(int argc, char* argv[])
{
  // INIT
  ThrowIfFailed(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));
  ThrowIfFailed(MFStartup(MF_VERSION));

  // Get devices.
  uint32_t deviceCount = 0;
  IMFActivate** devices = nullptr;
  {
    std::shared_ptr<IMFAttributes> pAttributes;
    IMFAttributes* pRawAttributes = nullptr;
    ThrowIfFailed(MFCreateAttributes(&pRawAttributes, 1));
    pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

    ThrowIfFailed(pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));

    ThrowIfFailed(MFEnumDeviceSources(pAttributes.get(), &devices, &deviceCount));

    for (uint32_t i = 0; i < deviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      ThrowIfFailed(devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length));
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }
  
  if (deviceCount == 0)
  {
    CoTaskMemFree(devices);
    ThrowIfFailed(MFShutdown());
    CoUninitialize();
    return -1;
  }

  // Create capturemanager.
  CaptureManager* g_pEngine = nullptr;
  ThrowIfFailed(CaptureManager::createInst(&g_pEngine));

  // Input device no.
  uint32_t selectionNo = 0;
  std::wcout << "Please input device no : ";
  std::wcin >> selectionNo;
  if (selectionNo < deviceCount)
  {
    ThrowIfFailed(g_pEngine->initCaptureManager(devices[selectionNo]));
  }
  else
  {
    std::wcout << "Failed device select.";
  }

  // window message handle
  Win32MessageHandler::getInstance().run((HINSTANCE)0, 1);

  // Release
  if (g_pEngine)
  {
    delete g_pEngine;
    g_pEngine = nullptr;
  }

  if (devices != nullptr)
  {
    for (uint32_t i = 0; i < deviceCount; i++)
    {
      devices[i]->Release();
    }
    CoTaskMemFree(devices);
  }

  ThrowIfFailed(MFShutdown());
  CoUninitialize();

  return 0;
}
