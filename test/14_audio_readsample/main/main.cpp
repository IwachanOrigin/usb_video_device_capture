
#include "stdafx.h"
#include "win32messagehandler.h"
#include "audiocapturemanager.h"
#include "audiooutputdevicemanager.h"

#include <locale>

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "wmcodecdspuuid.lib")

#pragma comment(lib, "SDL2")

#undef main // for SDL2

using namespace message_handler;

static inline bool getCaptureDevices(uint32_t& deviceCount, IMFActivate**& devices, bool audioMode = false)
{
  GUID searchGUID = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
  if (audioMode)
  {
    searchGUID = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
  }

  std::shared_ptr<IMFAttributes> pAttributes;
  IMFAttributes* pRawAttributes = nullptr;
  HRESULT hr = MFCreateAttributes(&pRawAttributes, 1);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set create attributes." << std::endl;
    return false;
  }
  pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

  hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, searchGUID);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set attribute to MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE." << std::endl;
    return false;
  }

  hr = MFEnumDeviceSources(pAttributes.get(), &devices, &deviceCount);
  if (FAILED(hr))
  {
    std::wcout << "Failed to initialize the media foundation." << std::endl;
    return false;
  }

  return true;
}

static inline void releaseAllDevices(IMFActivate**& devices, uint32_t& deviceCount)
{
  for (uint32_t i = 0; i < deviceCount; i++)
  {
    devices[i]->Release();
  }
  CoTaskMemFree(devices);
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

  // Set locale(use to the system default locale)
  std::wcout.imbue(std::locale(""));

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

  // Get video devices.
  uint32_t audioDeviceCount = 0;
  IMFActivate** audioDevices = nullptr;
  if (getCaptureDevices(audioDeviceCount, audioDevices, true))
  {
    for (uint32_t i = 0; i < audioDeviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      hr = audioDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length);
      if (FAILED(hr))
      {
        std::wcerr << "Failed to get audio device name." << std::endl;
        continue;
      }
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }

  if (audioDeviceCount == 0)
  {
    std::wcerr << "Audio Device Not Found." << std::endl;
    CoTaskMemFree(audioDevices);
    hr = MFShutdown();
    CoUninitialize();
    return -1;
  }

  // Input device no.
  uint32_t selectionNo = 0;

  std::wcout << "Please input device no : ";
  std::wcin >> selectionNo;
  if (selectionNo > audioDeviceCount)
  {
    std::wcerr << "Failed device select." << std::endl;
    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }
    hr = MFShutdown();
    CoUninitialize();
    return -1;
  }

  // Create main window.
  bool result = Win32MessageHandler::getInstance().init((HINSTANCE)0, 1);
  if (!result)
  {
    std::wcerr << "Failed to create main window." << std::endl;
    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }
    hr = MFShutdown();
    CoUninitialize();

    return -1;
  }

  // Setup audio device
  int errCode = AudioOutputDeviceManager::getInstance().init(4);
  if (errCode == 0)
  {
    errCode = AudioOutputDeviceManager::getInstance().start();
    if (errCode != 0)
    {
      std::wcout << "Failed to start audio device." << std::endl;
    }
  }
  else
  {
    std::wcout << "Failed to initialize the audio device." << std::endl;
  }

  // Create capturemanager.
  int retInt = AudioCaptureManager::getInstance().init(audioDevices[selectionNo]);
  if (retInt < 0)
  {
    std::wcerr << "Failed to init capture manager." << std::endl;
    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }
    hr = MFShutdown();
    CoUninitialize();

    return -1;
  }
  audioDevices[selectionNo]->AddRef();

  // Start message loop
  Win32MessageHandler::getInstance().run();

  // Release
  if (audioDevices != nullptr)
  {
    releaseAllDevices(audioDevices, audioDeviceCount);
  }
  hr = MFShutdown();
  CoUninitialize();

  return 0;
}
