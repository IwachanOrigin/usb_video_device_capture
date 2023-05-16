
#include "stdafx.h"
#include "dxhelper.h"
#include <locale>
#include "win32messagehandler.h"
#include "capturemanager.h"
#include "dx11manager.h"
#include "audiodevicemanager.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "powrprof")

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#pragma comment(lib, "SDL2")

#undef main // for SDL2

using namespace helper;
using namespace message_handler;
using namespace manager;

static inline void getVideoDevices(IMFActivate*** pppRawDevice, uint32_t& count)
{
  std::shared_ptr<IMFAttributes> pAttributes;
  IMFAttributes* pRawAttributes = nullptr;
  HRESULT hr = MFCreateAttributes(&pRawAttributes, 1);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create attributes.", L"Error", MB_OK);
    return;
  }
  pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

  hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE for attribute.", L"Error", MB_OK);
    return;
  }

  count = 0;
  IMFActivate** ppRawDevice = nullptr;
  hr = MFEnumDeviceSources(pAttributes.get(), &ppRawDevice, &count);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to Enumerate device sources.", L"Error", MB_OK);
    return;
  }

  for (uint32_t i = 0; i < count; i++)
  {
    wchar_t* buffer = nullptr;
    uint32_t length = 0;
    hr = ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length);
    if (FAILED(hr))
    {
      continue;
    }
    std::wcout << "No. " << i << " : " << buffer << std::endl;
  }
  pppRawDevice = &ppRawDevice;
}

/**
 * Gets a video source reader from a device such as a webcam.
 * @param[in] nDevice: the video device index to attempt to get the source reader for.
 * @param[out] ppVideoSource: will be set with the source for the reader if successful.
 * @param[out] ppVideoReader: will be set with the reader if successful. Set this parameter
 *  to nullptr if no reader is required and only the source is needed.
 * @@Returns S_OK if successful or an error code if not.
 */
static inline HRESULT GetVideoSourceFromDevice(UINT nDevice, IMFMediaSource** ppVideoSource, IMFSourceReader** ppVideoReader)
{
  UINT32 videoDeviceCount = 0;
  ComPtr<IMFAttributes> videoConfig = nullptr;
  IMFActivate** videoDevices = nullptr;
  ComPtr<IMFAttributes> pAttributes = nullptr;

  HRESULT hr = S_OK;

  // Get the first available webcam.
  hr = MFCreateAttributes(videoConfig.GetAddressOf(), 1);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create video configuration.", L"Error", MB_OK);
    return hr;
  }

  // Request video capture devices.
  hr = videoConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set GUID to video config.", L"Error", MB_OK);
    return hr;
  }

  hr = MFEnumDeviceSources(videoConfig.Get(), &videoDevices, &videoDeviceCount);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get device sources.", L"Error", MB_OK);
    return hr;
  }

  if (nDevice >= videoDeviceCount)
  {
    printf("The device index of %d was invalid for available device count of %d.\n", nDevice, videoDeviceCount);
    hr = E_INVALIDARG;
  }
  else
  {
    hr = videoDevices[nDevice]->ActivateObject(IID_PPV_ARGS(ppVideoSource));
    if (FAILED(hr))
    {
      MessageBoxW(nullptr, L"Failed to activate device..", L"Error", MB_OK);
      return hr;
    }

    hr = MFCreateAttributes(pAttributes.GetAddressOf(), 1);
    if (FAILED(hr))
    {
      MessageBoxW(nullptr, L"Failed to create attribute.", L"Error", MB_OK);
      return hr;
    }

    if (ppVideoReader != nullptr)
    {
      // Adding this attribute creates a video source reader that will handle
      // colour conversion and avoid the need to manually convert between RGB24 and RGB32 etc.
      hr = pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1);
      if (FAILED(hr))
      {
        MessageBoxW(nullptr, L"Failed to set MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING to attribute.", L"Error", MB_OK);
        return hr;
      }

      // Create a source reader.
      hr = MFCreateSourceReaderFromMediaSource(*ppVideoSource, pAttributes.Get(), ppVideoReader);
      if (FAILED(hr))
      {
        MessageBoxW(nullptr, L"Failed to MFCreateSourceReaderFromMediaSource.", L"Error", MB_OK);
        return hr;
      }
    }
  }

  // Device release
  if (videoDevices != nullptr)
  {
    for (uint32_t i = 0; i < videoDeviceCount; i++)
    {
      videoDevices[i]->Release();
    }
    CoTaskMemFree(videoDevices);
  }

  return hr;
}

static inline bool findMatchFormatTypes(const int& deviceIndex, const uint32_t& capWidth, const uint32_t& capHeight, const uint32_t& capFps)
{
  HRESULT hr = S_OK;
  ComPtr<IMFMediaSource> pVideoSource = nullptr;
  ComPtr<IMFSourceReader> pVideoReader = nullptr;
  ComPtr<IMFPresentationDescriptor> pSourcePresentationDescriptor = nullptr;
  BOOL fSelected = false;

  hr = GetVideoSourceFromDevice(deviceIndex, &pVideoSource, &pVideoReader);
  if (FAILED(hr))
  {
    std::wcout << "Failed to GetVideoSourceFromDevice func." << std::endl;
    return false;
  }
  hr = pVideoSource->CreatePresentationDescriptor(&pSourcePresentationDescriptor);
  if (FAILED(hr))
  {
    std::wcout << "Failed to create the presentation descriptor from the media source." << std::endl;
    return false;
  }

  DWORD streamDescCount = 0;
  hr = pSourcePresentationDescriptor->GetStreamDescriptorCount(&streamDescCount);
  if (FAILED(hr))
  {
    std::wcout << "Failed to get stream descriptor count." << std::endl;
    return false;
  }

  for (int descIndex = 0; descIndex < (int)streamDescCount; descIndex++)
  {
    ComPtr<IMFStreamDescriptor> pSourceStreamDescriptor = nullptr;
    hr = pSourcePresentationDescriptor->GetStreamDescriptorByIndex(descIndex, &fSelected, &pSourceStreamDescriptor);
    if (FAILED(hr))
    {
      std::wcout << "Failed to get source stream descriptor from presentation descriptor" << std::endl;
      return false;
    }

    ComPtr<IMFMediaTypeHandler> pSourceMediaTypeHandler = nullptr;
    hr = pSourceStreamDescriptor->GetMediaTypeHandler(&pSourceMediaTypeHandler);
    if (FAILED(hr))
    {
      std::wcout << "Failed to get source media type handler." << std::endl;
      return false;
    }

    DWORD typeCount = 0;
    hr = pSourceMediaTypeHandler->GetMediaTypeCount(&typeCount);
    if (FAILED(hr))
    {
      std::wcout << "Failed to get source media type count." << std::endl;
      return false;
    }

    ComPtr<IMFMediaType> wkMediaType = nullptr;
    hr = pSourceMediaTypeHandler->GetCurrentMediaType(wkMediaType.GetAddressOf());
    if (FAILED(hr))
    {
      std::wcout << "Failed to get current media type." << std::endl;
      return false;
    }

    hr = wkMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (FAILED(hr))
    {
      std::wcout << "Failed to set MF_MT_SUBTYPE MFVideoFormat_RGB32." << std::endl;
      return false;
    }

    hr = MFSetAttributeSize(wkMediaType.Get(), MF_MT_FRAME_SIZE, capWidth, capHeight);
    if (FAILED(hr))
    {
      std::wcout << "Failed to set the frame size attribute on media type." << std::endl;
      return false;
    }

    hr = MFSetAttributeRatio(wkMediaType.Get(), MF_MT_FRAME_RATE, capFps, 1);
    if (FAILED(hr))
    {
      std::wcout << "Failed to set the frame rate attribute on media type." << std::endl;
      return false;
    }

    hr = pSourceMediaTypeHandler->SetCurrentMediaType(wkMediaType.Get());
    if (SUCCEEDED(hr))
    {
      break;
    }
  }

  return true;
}

static inline bool getCaptureDevices(uint32_t& deviceCount, IMFActivate**& devices, bool audioMode = false)
{
  GUID searchGUID = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
  if (audioMode)
  {
    searchGUID = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
  }

  std::shared_ptr<IMFAttributes> pAttributes;
  IMFAttributes* pRawAttributes = nullptr;
  ThrowIfFailed(MFCreateAttributes(&pRawAttributes, 1));
  pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

  HRESULT hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, searchGUID);
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

int main(int argc, char* argv[])
{
  // Set locale(use to the system default locale)
  std::wcout.imbue(std::locale(""));

  // INIT
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (FAILED(hr))
  {
    std::wcout << "Failed to initialize the COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE" << std::endl;
    return -1;
  }
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    std::wcout << "Failed to initialize the media foundation." << std::endl;
    return -1;
  }

  // Get video devices.
  uint32_t videoDeviceCount = 0;
  IMFActivate** videoDevices = nullptr;
  if (getCaptureDevices(videoDeviceCount, videoDevices))
  {
    for (uint32_t i = 0; i < videoDeviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      ThrowIfFailed(videoDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length));
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }

  if (videoDeviceCount == 0)
  {
    std::wcout << "Video Device Not Found." << std::endl;
    CoTaskMemFree(videoDevices);
    ThrowIfFailed(MFShutdown());
    CoUninitialize();
    return -1;
  }

  // Input video device no.
  uint32_t videoSelectionNo = 0;
  std::wcout << "Please input video device no : ";
  std::wcin >> videoSelectionNo;
  std::wcout << std::endl;
  if (videoSelectionNo > videoDeviceCount)
  {
    std::wcout << "Failed video device select.";
    if (videoDevices != nullptr)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }
    return -1;
  }

  // Get audio devices.
  uint32_t audioDeviceCount = 0;
  IMFActivate** audioDevices = nullptr;
  if (getCaptureDevices(audioDeviceCount, audioDevices, true))
  {
    for (uint32_t i = 0; i < audioDeviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      ThrowIfFailed(audioDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length));
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }

  // Input video device no.
  uint32_t audioSelectionNo = 0;
  if (audioDeviceCount != 0)
  {
    std::wcout << "Please input audio device no : ";
    std::wcin >> audioSelectionNo;
    std::wcout << std::endl;
    if (audioSelectionNo > audioDeviceCount)
    {
      std::wcout << "Failed audio device select.";
      if (audioDevices != nullptr)
      {
        releaseAllDevices(audioDevices, audioDeviceCount);
      }
      return -1;
    }
  }

  // Create capturemanager.
  CaptureManager* g_pEngine = nullptr;
  ThrowIfFailed(CaptureManager::createInst(&g_pEngine));

  // Initialize capture manager.
  hr = g_pEngine->initCaptureManager(videoDevices[videoSelectionNo], audioDevices[audioSelectionNo]);
  if (FAILED(hr))
  {
    std::wcout << "Failed to init capture manager." << std::endl;
    return -1;
  }
  videoDevices[videoSelectionNo]->AddRef();

  // Input capture size of width, height, fps
  uint32_t capWidth = 0, capHeight = 0, capFps = 0;
  std::wcout << "Please input capture size of width, height, fps." << std::endl;
  std::wcout << "ex. 3840 2160 30" << std::endl;
  std::wcout << " > ";
  std::wcin >> capWidth >> capHeight >> capFps;
  std::wcout << std::endl;
  // Check whether the selected USB device supports the input resolution and frame rate.
  bool result = findMatchFormatTypes(videoSelectionNo, capWidth, capHeight, capFps);
  if (!result)
  {
    std::wcout << "No matching format." << std::endl;
    if (videoDevices != nullptr)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }

    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }

    return -1;
  }

  uint32_t windowWidth = 0, windowHeight = 0;
  std::wcout << "Please input Display size of window width and height." << std::endl;
  std::wcout << "ex. 1920 1080" << std::endl;
  std::wcout << " > ";
  std::wcin >> windowWidth >> windowHeight;

  // Create main window.
  result = Win32MessageHandler::getInstance().init((HINSTANCE)0, 1, windowWidth, windowHeight);
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

    MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK);
    return -1;
  }

  // Get HWND
  HWND previewWnd = Win32MessageHandler::getInstance().hwnd();

  HPOWERNOTIFY hPowerNotify = nullptr;
  HPOWERNOTIFY hPowerNotifyMonitor = nullptr;
  SYSTEM_POWER_CAPABILITIES pwrCaps{};
  // Information cannot be obtained from the device without the following process.
  hPowerNotify = RegisterSuspendResumeNotification((HANDLE)previewWnd, DEVICE_NOTIFY_WINDOW_HANDLE);
  hPowerNotifyMonitor = RegisterPowerSettingNotification((HANDLE)previewWnd, &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);
  ZeroMemory(&pwrCaps, sizeof(pwrCaps));
  GetPwrCapabilities(&pwrCaps);

  // Create dx11 device, context, swapchain
  result = DX11Manager::getInstance().init(previewWnd, capWidth, capHeight, capFps);
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
    return -1;
  }

  // Setup audio device
  int errCode = AudioDeviceManager::getInstance().init(2);
  if (errCode == 0)
  {
    errCode = AudioDeviceManager::getInstance().start();
    if (errCode != 0)
    {
      std::wcout << "Failed to start audio device." << std::endl;
    }
  }
  else
  {
    std::wcout << "Failed to initialize the audio device." << std::endl;
  }

  // Start preview
  hr = g_pEngine->startPreview(capWidth, capHeight, capFps);
  if (FAILED(hr))
  {
    std::wcout << "Failed to start device preview." << std::endl;
    if (videoDevices != nullptr)
    {
      releaseAllDevices(videoDevices, videoDeviceCount);
    }

    if (audioDevices != nullptr)
    {
      releaseAllDevices(audioDevices, audioDeviceCount);
    }
    return -1;
  }

  // Start message loop
  Win32MessageHandler::getInstance().run();

  // Stop preview
  ThrowIfFailed(g_pEngine->stopPreview());

  // Stop audio
  errCode = AudioDeviceManager::getInstance().stop();
  if (errCode != 0)
  {
    std::wcout << "Failed to stop audio device." << std::endl;
  }

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
