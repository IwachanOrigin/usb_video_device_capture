
#include "stdafx.h"
#include "win32messagehandler.h"
#include "capturemanager.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "wmcodecdspuuid.lib")

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

using namespace message_handler;

void getVideoDevices(IMFActivate*** pppRawDevice, uint32_t& count)
{
  std::shared_ptr<IMFAttributes> pAttributes;
  IMFAttributes* pRawAttributes = nullptr;
  HRESULT hr = S_OK;
  hr = MFCreateAttributes(&pRawAttributes, 1);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to create attributes." << std::endl;
    count = 0;
    return;
  }
  pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

  hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to set guid." << std::endl;
    count = 0;
    return;
  }

  count = 0;
  IMFActivate** ppRawDevice = nullptr;
  hr = MFEnumDeviceSources(pAttributes.get(), &ppRawDevice, &count);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to enum rate device sorces." << std::endl;
    return;
  }

  for (uint32_t i = 0; i < count; i++)
  {
    wchar_t* buffer = nullptr;
    uint32_t length = 0;
    hr = ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length);
    if (FAILED(hr))
    {
      std::wcerr << "Failed to get device name." << std::endl;
      continue;
    }
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
  const wchar_t windowClassName[100] = L"Capture Engine Preview Window Class";

  WNDCLASSEXW wcex = {};

  wcex.lpfnWndProc = WindowProc;
  wcex.hInstance = hInstance;
  wcex.lpszClassName = windowClassName;

  RegisterClassExW(&wcex);

  // Create the window.
  return CreateWindowExW(
    0
    , windowClassName
    , L"Capture Application"
    , WS_OVERLAPPEDWINDOW
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , CW_USEDEFAULT
    , nullptr
    , nullptr
    , hInstance
    , nullptr);
}

int main(int argc, char* argv[])
{
  HRESULT hr = S_OK;
  // INIT
  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to CoInitializeEx." << std::endl;
    return -1;
  }
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    std::wcerr << "Failed to MFStartup." << std::endl;
    return -1;
  }

  // Get devices.
  uint32_t deviceCount = 0;
  IMFActivate** devices = nullptr;
  getVideoDevices(&devices, deviceCount);
  if (deviceCount == 0)
  {
    CoTaskMemFree(devices);
    hr = MFShutdown();
    CoUninitialize();
    return -1;
  }

  // Input device no.
  uint32_t selectionNo = 0;

  std::wcout << "Please input device no : ";
  std::wcin >> selectionNo;
  if (selectionNo > deviceCount)
  {
    std::wcout << "Failed device select.";
    if (devices != nullptr)
    {
      for (uint32_t i = 0; i < deviceCount; i++)
      {
        devices[i]->Release();
      }
      CoTaskMemFree(devices);
    }
    hr = MFShutdown();
    CoUninitialize();
    return -1;
  }

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
    hr = MFShutdown();
    CoUninitialize();

    MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK);
    return -1;
  }

  // Create capturemanager.
  int retInt = CaptureManager::getInstance().init(devices[selectionNo]);
  if (retInt < 0)
  {
    if (devices != nullptr)
    {
      for (uint32_t i = 0; i < deviceCount; i++)
      {
        devices[i]->Release();
      }
      CoTaskMemFree(devices);
    }
    hr = MFShutdown();
    CoUninitialize();

    MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK);
    return -1;
  }
  devices[selectionNo]->AddRef();

  // Start message loop
  Win32MessageHandler::getInstance().run();

  // Release
  if (devices != nullptr)
  {
    for (uint32_t i = 0; i < deviceCount; i++)
    {
      devices[i]->Release();
    }
    CoTaskMemFree(devices);
  }
  hr = MFShutdown();
  CoUninitialize();

  return 0;
}
