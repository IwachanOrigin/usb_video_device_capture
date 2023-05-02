
#include "stdafx.h"
#include "dxhelper.h"
#include "win32messagehandler.h"
#include "capturemanager.h"
#include "dx11manager.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "powrprof")

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

using namespace helper;
using namespace message_handler;
using namespace manager;

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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_ERASEBKGND:
    return 1;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

  HPOWERNOTIFY hPowerNotify = nullptr;
  HPOWERNOTIFY hPowerNotifyMonitor = nullptr;
  SYSTEM_POWER_CAPABILITIES pwrCaps{};

  std::wcout << "Please input device no : ";
  std::wcin >> selectionNo;
  std::wcout << std::endl;
  if (selectionNo > deviceCount)
  {
    std::wcout << "Failed device select.";
    return -1;
  }

  // Input capture size of width, height, fps
  uint32_t capWidth = 0, capHeight = 0, capFps = 0;
  std::wcout << "Please input capture size of width, height, fps." << std::endl;
  std::wcout << "ex. 3840 2160 30" << std::endl;
  std::wcout << " > ";
  std::wcin >> capWidth >> capHeight >> capFps;
  std::wcout << std::endl;
  // TODO : Check whether the selected USB device supports the input resolution and frame rate.
  //

  uint32_t windowWidth = 0, windowHeight = 0;
  std::wcout << "Please input Display size of window width and height." << std::endl;
  std::wcout << "ex. 1920 1080" << std::endl;
  std::wcout << " > ";
  std::wcin >> windowWidth >> windowHeight;

  ThrowIfFailed(g_pEngine->initCaptureManager(devices[selectionNo]));
  devices[selectionNo]->AddRef();

  // Create main window.
  bool result = Win32MessageHandler::getInstance().init((HINSTANCE)0, 1);
  if (!result)
  {
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

    MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK);
    return -1;
  }

  HWND previewWnd = Win32MessageHandler::getInstance().hwnd();
  // Information cannot be obtained from the device without the following process.
  hPowerNotify = RegisterSuspendResumeNotification((HANDLE)previewWnd, DEVICE_NOTIFY_WINDOW_HANDLE);
  hPowerNotifyMonitor = RegisterPowerSettingNotification((HANDLE)previewWnd, &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);
  ZeroMemory(&pwrCaps, sizeof(pwrCaps));
  GetPwrCapabilities(&pwrCaps);

  // Start preview
  ThrowIfFailed(g_pEngine->startPreview());

  // Create dx11 device, context, swapchain  
  result = DX11Manager::getInstance().init(previewWnd);
  if (!result)
  {
    ThrowIfFailed(g_pEngine->stopPreview());

    if (g_pEngine)
    {
      delete g_pEngine;
      g_pEngine = nullptr;
    }

    if (hPowerNotify)
    {
      UnregisterSuspendResumeNotification(hPowerNotify);
      hPowerNotify = NULL;
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
  }

  // Start message loop
  Win32MessageHandler::getInstance().run();

  // Stop preview
  ThrowIfFailed(g_pEngine->stopPreview());

  // Release
  if (g_pEngine)
  {
    delete g_pEngine;
    g_pEngine = nullptr;
  }

  if (hPowerNotify)
  {
    UnregisterSuspendResumeNotification(hPowerNotify);
    hPowerNotify = NULL;
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
