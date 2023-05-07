
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

void getVideoDevices(IMFActivate*** pppRawDevice, uint32_t& count)
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
HRESULT GetVideoSourceFromDevice(UINT nDevice, IMFMediaSource** ppVideoSource, IMFSourceReader** ppVideoReader)
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

bool findMatchFormatTypes(const int& deviceIndex, const uint32_t& capWidth, const uint32_t& capHeight, const uint32_t& capFps)
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

    for (int typeIndex = 0; typeIndex < (int)typeCount; typeIndex++)
    {
      ComPtr<IMFMediaType> pMediaType = nullptr;
      hr = pSourceMediaTypeHandler->GetMediaTypeByIndex(typeIndex, pMediaType.GetAddressOf());
      if (FAILED(hr))
      {
        // "Error retrieving media type."
        continue;
      }

      uint32_t width = 0, height = 0;
      hr = MFGetAttributeSize(pMediaType.Get(), MF_MT_FRAME_SIZE, &width, &height);
      if (FAILED(hr))
      {
        // "Failed to get the frame size attribute on media type."
        continue;
      }

      GUID formatType{};
      hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &formatType);
      if (FAILED(hr))
      {
        // "Failed to get the subtype guid on media type."
        continue;
      }

      uint32_t interlaceMode = 0;
      hr = pMediaType->GetUINT32(MF_MT_INTERLACE_MODE, &interlaceMode);
      if (FAILED(hr))
      {
        // "Failed to get the interlace mode on media type."
        continue;
      }

      uint32_t fpsNum = 0, fpsDen = 0;
      hr = MFGetAttributeRatio(pMediaType.Get(), MF_MT_FRAME_RATE, &fpsNum, &fpsDen);
      if (FAILED(hr))
      {
        // "Failed to get the frame rate on media type."
        continue;
      }

      // Check it!
      if(IsEqualGUID(MFVideoFormat_RGB32, formatType) && width == capWidth && height == capHeight && fpsNum == capFps && fpsDen == 1)
      {
        hr = S_OK;
        break;
      }
    }
  }

  return true;
}

bool getCaptureDevices(uint32_t& deviceCount, IMFActivate**& devices, bool audioMode = false)
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
#if 0
  {
    std::shared_ptr<IMFAttributes> pAttributes;
    IMFAttributes* pRawAttributes = nullptr;
    ThrowIfFailed(MFCreateAttributes(&pRawAttributes, 1));
    pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
      std::wcout << "Failed to set attribute to MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE." << std::endl;
      return -1;
    }

    hr = MFEnumDeviceSources(pAttributes.get(), &videoDevices, &videoDeviceCount);
    if (FAILED(hr))
    {
      std::wcout << "Failed to initialize the media foundation." << std::endl;
      return -1;
    }

    for (uint32_t i = 0; i < videoDeviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      ThrowIfFailed(videoDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length));
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }
#else
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
#endif

  if (videoDeviceCount == 0)
  {
    CoTaskMemFree(videoDevices);
    ThrowIfFailed(MFShutdown());
    CoUninitialize();
    return -1;
  }

  // Get audio devices.
  uint32_t audioDeviceCount = 0;
  IMFActivate** audioDevices = nullptr;
  {
    std::shared_ptr<IMFAttributes> pAttributes;
    IMFAttributes* pRawAttributes = nullptr;
    ThrowIfFailed(MFCreateAttributes(&pRawAttributes, 1));
    pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p) { p->Release(); });

    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED(hr))
    {
      std::wcout << "Failed to set attribute to MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE." << std::endl;
      return -1;
    }

    hr = MFEnumDeviceSources(pAttributes.get(), &audioDevices, &audioDeviceCount);
    if (FAILED(hr))
    {
      std::wcout << "Failed to initialize the media foundation." << std::endl;
      return -1;
    }

    for (uint32_t i = 0; i < audioDeviceCount; i++)
    {
      wchar_t* buffer = nullptr;
      uint32_t length = 0;
      ThrowIfFailed(audioDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length));
      std::wcout << "No. " << i << " : " << buffer << std::endl;
    }
  }

  if (audioDeviceCount == 0)
  {
    std::wcout << "Failed to get audio devices." << std::endl;
    CoTaskMemFree(audioDevices);
    ThrowIfFailed(MFShutdown());
    CoUninitialize();
    return -1;
  }

  if (audioDevices != nullptr)
  {
    for (uint32_t i = 0; i < audioDeviceCount; i++)
    {
      audioDevices[i]->Release();
    }
    CoTaskMemFree(audioDevices);
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
  if (selectionNo > videoDeviceCount)
  {
    std::wcout << "Failed device select.";
    return -1;
  }

  // Initialize capture manager.
  hr = g_pEngine->initCaptureManager(videoDevices[selectionNo]);
  if (FAILED(hr))
  {
    std::wcout << "Failed to init capture manager." << std::endl;
    return -1;
  }
  videoDevices[selectionNo]->AddRef();

  // Input capture size of width, height, fps
  uint32_t capWidth = 0, capHeight = 0, capFps = 0;
  std::wcout << "Please input capture size of width, height, fps." << std::endl;
  std::wcout << "ex. 3840 2160 30" << std::endl;
  std::wcout << " > ";
  std::wcin >> capWidth >> capHeight >> capFps;
  std::wcout << std::endl;
  // Check whether the selected USB device supports the input resolution and frame rate.
  bool result = findMatchFormatTypes(selectionNo, capWidth, capHeight, capFps);
  if (!result)
  {
    std::wcout << "No matching format." << std::endl;
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
      for (uint32_t i = 0; i < videoDeviceCount; i++)
      {
        videoDevices[i]->Release();
      }
      CoTaskMemFree(videoDevices);
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
      for (uint32_t i = 0; i < videoDeviceCount; i++)
      {
        videoDevices[i]->Release();
      }
      CoTaskMemFree(videoDevices);
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
    for (uint32_t i = 0; i < videoDeviceCount; i++)
    {
      videoDevices[i]->Release();
    }
    CoTaskMemFree(videoDevices);
  }

  ThrowIfFailed(MFShutdown());
  CoUninitialize();

  return 0;
}
