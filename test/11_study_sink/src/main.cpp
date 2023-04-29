
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_ERASEBKGND:
    return 1;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreatePreviewWindow(HINSTANCE hInstance, HWND hParent)
{
  // Register the window class.
  const wchar_t CLASS_NAME[] = L"Capture Engine Preview Window Class";

  WNDCLASS wc = { };

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass(&wc);

  // Create the window.
  return CreateWindowEx(0, CLASS_NAME, L"Capture Application",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL, hInstance, NULL);
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
  SYSTEM_POWER_CAPABILITIES pwrCaps;
  HWND previewWnd;

  std::wcout << "Please input device no : ";
  std::wcin >> selectionNo;
  if (selectionNo < deviceCount)
  {
    ThrowIfFailed(g_pEngine->initCaptureManager(devices[selectionNo]));
    devices[selectionNo]->AddRef();

    previewWnd = CreatePreviewWindow(GetModuleHandle(NULL), nullptr);
    // Information cannot be obtained from the device without the following process.
    hPowerNotify = RegisterSuspendResumeNotification((HANDLE)previewWnd, DEVICE_NOTIFY_WINDOW_HANDLE);
    hPowerNotifyMonitor = RegisterPowerSettingNotification((HANDLE)previewWnd, &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);
    ZeroMemory(&pwrCaps, sizeof(pwrCaps));
    GetPwrCapabilities(&pwrCaps);
  }
  else
  {
    std::wcout << "Failed device select.";
  }

  // Start preview
  ThrowIfFailed(g_pEngine->startPreview());

  // window message handle
  Win32MessageHandler::getInstance().run((HINSTANCE)0, 1);

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
