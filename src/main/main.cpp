
#include "stdafx.h"
#include "dxhelper.h"
#include "win32messagehandler.h"
#include "capturemanager.h"
#include "audiocapturemanager.h"
#include "audiooutputdevicemanager.h"
#include "dx11manager.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "wmcodecdspuuid.lib")

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#pragma comment(lib, "SDL2")

#undef main // for SDL2

using namespace helper;
using namespace message_handler;
using namespace manager;

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
  ThrowIfFailed(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));
  ThrowIfFailed(MFStartup(MF_VERSION));

  // Get video devices.
  uint32_t videoDeviceCount = 0;
  IMFActivate** videoDevices = nullptr;
  std::wcout << "----- Video Devices -----" << std::endl;
  if (getCaptureDevices(videoDeviceCount, videoDevices))
  {
    for (uint32_t i = 0; i < videoDeviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      hr = videoDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length);
      if (FAILED(hr))
      {
        std::wcerr << "Failed to get video device name." << std::endl;
        continue;
      }
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }

  if (videoDeviceCount == 0)
  {
    std::wcerr << "Video Device not found." << std::endl;
    CoTaskMemFree(videoDevices);
    ThrowIfFailed(MFShutdown());
    CoUninitialize();
    return -1;
  }

  // Input device no.
  uint32_t videoSelectionNo = 0;

  std::wcout << "Please input video device no : ";
  std::wcin >> videoSelectionNo;
  if (videoSelectionNo > videoDeviceCount)
  {
    std::wcout << "Failed video device select.";
    return -1;
  }

  // Get audio devices.
  uint32_t audioDeviceCount = 0;
  IMFActivate** audioDevices = nullptr;
  std::wcout << "----- Audio Devices -----" << std::endl;
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
    std::wcerr << "Audio Device not found." << std::endl;
    if (!videoDevices)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }
    CoTaskMemFree(audioDevices);
    ThrowIfFailed(MFShutdown());
    CoUninitialize();
    return -1;
  }

  // Input audio device no.
  uint32_t audioSelectionNo = 0;
  std::wcout << "Please input audio device no : ";
  std::wcin >> audioSelectionNo;
  if (audioSelectionNo > audioDeviceCount)
  {
    std::wcout << "Failed video device select.";
    return -1;
  }

  // Create main window.
  bool result = Win32MessageHandler::getInstance().init((HINSTANCE)0, 1);
  if (!result)
  {
    if (videoDevices != nullptr)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }
    ThrowIfFailed(MFShutdown());
    CoUninitialize();

    MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK);
    return -1;
  }

  // Create capturemanager.
  int retInt = CaptureManager::getInstance().init(videoDevices[videoSelectionNo]);
  if (retInt < 0)
  {
    if (videoDevices != nullptr)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }
    ThrowIfFailed(MFShutdown());
    CoUninitialize();

    MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK);
    return -1;
  }
  videoDevices[videoSelectionNo]->AddRef();

  HWND previewWnd = Win32MessageHandler::getInstance().hwnd();
  uint32_t width = 0, height = 0, fps = 0;
  width = CaptureManager::getInstance().getCaptureWidth();
  height = CaptureManager::getInstance().getCaptureHeight();
  fps = CaptureManager::getInstance().getCaptureFps();
  // Create dx11 device, context, swapchain
  result = DX11Manager::getInstance().init(previewWnd, width, height, fps);
  if (!result)
  {
    if (videoDevices != nullptr)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }

    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }

    ThrowIfFailed(MFShutdown());
    CoUninitialize();
  }

  // Create audio capturemanager
  retInt = AudioCaptureManager::getInstance().init(audioDevices[audioSelectionNo]);
  if (retInt < 0)
  {
    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }

    MessageBoxW(nullptr, L"Failed to create audio capture manager.", L"Error", MB_OK);
  }
  else
  {
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
    audioDevices[audioSelectionNo]->AddRef();
  }

  // Start message loop
  Win32MessageHandler::getInstance().run();

  // Release
  if (videoDevices != nullptr)
  {
    releaseAllDevices(videoDevices, videoDeviceCount);
  }

  if (audioDevices != nullptr)
  {
    releaseAllDevices(audioDevices, audioDeviceCount);
  }
  ThrowIfFailed(MFShutdown());
  CoUninitialize();

  return 0;
}
