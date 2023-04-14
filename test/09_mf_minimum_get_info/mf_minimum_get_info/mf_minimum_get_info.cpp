// mf_minimum_get_info.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <guiddef.h>

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <string>

#pragma comment (lib, "mf.lib")
#pragma comment (lib, "mfplat.lib")
#pragma comment (lib, "mfuuid.lib")
#pragma comment (lib, "mfreadwrite.lib")

// GUIDの比較演算子を定義
bool operator<(const GUID& lhs, const GUID& rhs)
{
  return memcmp(&lhs, &rhs, sizeof(GUID)) < 0;
}

std::wstring GUIDToFormatName(const GUID& guid)
{
  // メディアフォーマットと対応するGUIDのマップ
  const std::map<GUID, std::wstring> guidToFormatNameMap = {
      {MFVideoFormat_RGB32, L"RGB32"},
      {MFVideoFormat_RGB24, L"RGB24"},
      {MFVideoFormat_NV12, L"NV12"},
      {MFVideoFormat_UYVY, L"UYVY"},
      {MFVideoFormat_I420, L"I420"},
      {MFAudioFormat_PCM, L"PCM"},
      {MFAudioFormat_Float, L"Float"},
      {MFAudioFormat_MP3, L"MP3"},
      {MFAudioFormat_AAC, L"AAC"},
      {MFAudioFormat_WMAudioV8, L"WMAudioV8"},
      {MFAudioFormat_Dolby_AC3, L"Dolby AC3"},
      {MFVideoFormat_AI44, L"AI44"},
      {MFVideoFormat_ARGB32, L"ARGB32"},
      {MFVideoFormat_AYUV, L"AYUV"},
      {MFVideoFormat_DV25, L"dv25"},
      {MFVideoFormat_DV50, L"dv50"},
      {MFVideoFormat_DVH1, L"dvh1"},
      {MFVideoFormat_DVSD, L"dvsd"},
      {MFVideoFormat_DVSL, L"dvsl"},
      {MFVideoFormat_H264, L"H264"},
      {MFVideoFormat_I420, L"I420"},
      {MFVideoFormat_IYUV, L"IYUV"},
      {MFVideoFormat_M4S2, L"M4S2"},
      {MFVideoFormat_MJPG, L"MJPG"},
      {MFVideoFormat_MP43, L"MP43"},
      {MFVideoFormat_MP4S, L"MP4S"},
      {MFVideoFormat_MP4V, L"MP4V"},
      {MFVideoFormat_MPG1, L"MPG1"},
      {MFVideoFormat_MSS1, L"MSS1"},
      {MFVideoFormat_MSS2, L"MSS2"},
      {MFVideoFormat_NV11, L"NV11"},
      {MFVideoFormat_NV12, L"NV12"},
      {MFVideoFormat_P010, L"P010"},
      {MFVideoFormat_P016, L"P016"},
      {MFVideoFormat_P210, L"P210"},
      {MFVideoFormat_P216, L"P216"},
      {MFVideoFormat_RGB555, L"RGB555"},
      {MFVideoFormat_RGB565, L"RGB565"},
      {MFVideoFormat_RGB8, L"RGB8"},
      {MFVideoFormat_UYVY, L"UYVY"},
      {MFVideoFormat_v210, L"v210"},
      {MFVideoFormat_v410, L"v410"},
      {MFVideoFormat_WMV1, L"WMV1"},
      {MFVideoFormat_WMV2, L"WMV2"},
      {MFVideoFormat_WMV3, L"WMV3"},
      {MFVideoFormat_WVC1, L"WVC1"},
      {MFVideoFormat_Y210, L"Y210"},
      {MFVideoFormat_Y216, L"Y216"},
      {MFVideoFormat_Y410, L"Y410"},
      {MFVideoFormat_Y416, L"Y416"},
      {MFVideoFormat_Y41P, L"Y41P"},
      {MFVideoFormat_Y41T, L"Y41T"},
      {MFVideoFormat_YUY2, L"YUY2"},
      {MFVideoFormat_YV12, L"YV12"},
      {MFVideoFormat_YVYU, L"YVYU"},
  };

  auto it = guidToFormatNameMap.find(guid);
  if (it != guidToFormatNameMap.end()) {
    return it->second;
  }
  else {
    return L"Unknown";
  }
}

static HRESULT InitializeMF()
{
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    hr = MFStartup(MF_VERSION);
  }
  return hr;
}

static HRESULT EnumerateCameraDevices(std::vector<IMFActivate*>& devices)
{
  HRESULT hr;

  std::shared_ptr<IMFAttributes> pAttributes;
  {
    IMFAttributes* prawAttributes = nullptr;
    hr = MFCreateAttributes(&prawAttributes, 1);
    if (FAILED(hr))
    {
      return hr;
    }

    pAttributes = std::shared_ptr<IMFAttributes>(prawAttributes, [](auto* p) { p->Release(); });
  }

  hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr))
  {
    return hr;
  }

  UINT32 count = 0;
  IMFActivate** ppDevices = nullptr;
  hr = MFEnumDeviceSources(pAttributes.get(), &ppDevices, &count);
  if (FAILED(hr))
  {
    return hr;
  }

  devices.assign(ppDevices, ppDevices + count);
  CoTaskMemFree(ppDevices);
  return hr;
}

static HRESULT GetAllMediaTypes(IMFActivate* pActivate, std::vector<IMFMediaType*>& mediaTypes)
{
  IMFMediaSource* pSource = NULL;
  IMFSourceReader* pReader = NULL;
  IMFMediaType* pMediaType = NULL;
  HRESULT hr = pActivate->ActivateObject(__uuidof(IMFMediaSource), (void**)&pSource);
  if (FAILED(hr)) {
    goto done;
  }

  hr = MFCreateSourceReaderFromMediaSource(pSource, NULL, &pReader);
  if (FAILED(hr)) {
    goto done;
  }

  for (DWORD i = 0;; i++) {
    hr = pReader->GetNativeMediaType(0, i, &pMediaType);
    if (hr == MF_E_NO_MORE_TYPES) {
      hr = S_OK;
      break;
    }
    else if (FAILED(hr)) {
      goto done;
    }
    mediaTypes.push_back(pMediaType);
    pMediaType = NULL;
  }

done:
  if (pMediaType) pMediaType->Release();
  if (pReader) pReader->Release();
  if (pSource) pSource->Release();
  return hr;
}


static HRESULT ReadVideoFrame(IMFSourceReader* pReader, IMFSample** ppSample)
{
  HRESULT hr = S_OK;

  while (true) {
    DWORD streamIndex, flags;
    LONGLONG timestamp;
    hr = pReader->ReadSample(
      MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      0, // No flags
      &streamIndex,
      &flags,
      &timestamp,
      ppSample
    );

    if (FAILED(hr)) {
      std::cerr << "Failed to read video frame" << std::endl;
      break;
    }

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
      hr = S_FALSE; // End of stream
      break;
    }

    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
      // Media type changed, continue reading
      continue;
    }

    if (flags & MF_SOURCE_READERF_STREAMTICK) {
      // Stream tick, continue reading
      continue;
    }

    if (*ppSample) {
      // Frame successfully read
      break;
    }
  }

  return hr;
}



static HRESULT SetMediaType(IMFActivate* pActivate, IMFMediaType* pMediaType, IMFSourceReader** ppReader)
{
  IMFMediaSource* pSource = NULL;
  HRESULT hr = MFCreateDeviceSource(pActivate, &pSource);
  if (FAILED(hr)) {
    goto done;
  }

  hr = MFCreateSourceReaderFromMediaSource(pSource, NULL, ppReader);
  if (FAILED(hr)) {
    goto done;
  }

  hr = (*ppReader)->SetCurrentMediaType(0, NULL, pMediaType);

done:
  if (pSource) pSource->Release();
  return hr;
}

static HRESULT GetCurrentMediaTypeFromReader(IMFSourceReader* pReader, IMFMediaType** ppMediaType)
{
  return pReader->GetCurrentMediaType(
    0, // Stream index: 0 is the default video stream
    ppMediaType // The IMFMediaType interface of the current media type
  );
}

static HRESULT SaveFrameAsBMP(IMFSample* pSample, const std::wstring& outputFilename, UINT32 width, UINT32 height)
{
  IMFMediaBuffer* pBuffer = NULL;
  BYTE* pImageData = NULL;
  DWORD imageDataLength = 0;

  HRESULT hr = pSample->ConvertToContiguousBuffer(&pBuffer);
  if (FAILED(hr)) {
    if (pImageData && pBuffer) {
      pBuffer->Unlock();
    }
    if (pBuffer) {
      pBuffer->Release();
    }

    return hr;
  }

  hr = pBuffer->Lock(&pImageData, NULL, &imageDataLength);
  if (FAILED(hr)) {
    if (pImageData && pBuffer) {
      pBuffer->Unlock();
    }
    if (pBuffer) {
      pBuffer->Release();
    }

    return hr;
  }

  BITMAPFILEHEADER bmpFileHeader = { 0 };
  BITMAPINFOHEADER bmpInfoHeader = { 0 };

  bmpFileHeader.bfType = 0x4D42; // 'BM'
  bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageDataLength;
  bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmpInfoHeader.biWidth = width;
  bmpInfoHeader.biHeight = -static_cast<LONG>(height); // Top-down image
  bmpInfoHeader.biPlanes = 1;
  bmpInfoHeader.biBitCount = 32; // Assume 32-bit input
  bmpInfoHeader.biCompression = BI_RGB;

  std::ofstream outputFile(outputFilename, std::ios::binary);
  outputFile.write(reinterpret_cast<const char*>(&bmpFileHeader), sizeof(BITMAPFILEHEADER));
  outputFile.write(reinterpret_cast<const char*>(&bmpInfoHeader), sizeof(BITMAPINFOHEADER));
  outputFile.write(reinterpret_cast<const char*>(pImageData), imageDataLength);

  if (pImageData && pBuffer) {
    pBuffer->Unlock();
  }
  if (pBuffer) {
    pBuffer->Release();
  }

  return hr;
}


int main()
{
  HRESULT hr = InitializeMF();
  if (FAILED(hr)) {
    std::cerr << "Failed to initialize Media Foundation" << std::endl;
    return -1;
  }

  std::vector<IMFActivate*> devices;
  hr = EnumerateCameraDevices(devices);
  if (FAILED(hr)) {
    std::cerr << "Failed to enumerate devices" << std::endl;
    MFShutdown();
    CoUninitialize();
    return -1;
  }

  if (devices.empty()) {
    std::cout << "No devices found" << std::endl;
    MFShutdown();
    CoUninitialize();
    return 0;
  }

  std::vector<IMFMediaType*> mediaTypes;
  hr = GetAllMediaTypes(devices[0], mediaTypes);
  if (FAILED(hr)) {
    std::cerr << "Failed to get all media types" << std::endl;
    MFShutdown();
    CoUninitialize();
    return -1;
  }

  // 最初に見つかったカメラデバイスで、最初に見つかったMediaTypeを設定します。
  // 必要に応じて、他のカメラデバイスや異なるMediaTypeに対しても設定できます。
  IMFSourceReader* pReader = NULL;
  hr = SetMediaType(devices[0], mediaTypes[532], &pReader);
  if (FAILED(hr)) {
    std::cerr << "Failed to set media type" << std::endl;
    MFShutdown();
    CoUninitialize();
    return -1;
  }

  // MediaTypeが設定された後に実行する処理をここに追加します。
  // 例: ビデオフレームの取得、表示、録画など
  IMFMediaType* pCurrentMediaType = NULL;
  hr = GetCurrentMediaTypeFromReader(pReader, &pCurrentMediaType);
  if (SUCCEEDED(hr))
  {
    UINT32 width, height;
    hr = MFGetAttributeSize(pCurrentMediaType, MF_MT_FRAME_SIZE, &width, &height);
    if (SUCCEEDED(hr)) {
      std::cout << "Current media type dimensions: " << width << "x" << height << std::endl;
    }

    UINT32 frameRateNumerator, frameRateDenominator;
    hr = MFGetAttributeRatio(pCurrentMediaType, MF_MT_FRAME_RATE, &frameRateNumerator, &frameRateDenominator);
    if (SUCCEEDED(hr)) {
      std::cout << "Current media type frame rate: " << frameRateNumerator << "/" << frameRateDenominator << std::endl;
    }

    GUID videoFormat{};
    if (SUCCEEDED(pCurrentMediaType->GetGUID(MF_MT_SUBTYPE, &videoFormat)))
    {
      WCHAR formatString[40] = { 0 };
      StringFromGUID2(videoFormat, formatString, 40);
      std::wcout << L"Video format: " << formatString << std::endl;
    }
  }
  else
  {
    std::cerr << "Failed to get current media type" << std::endl;
  }

  // 最初のフレームを取得してBMPファイルとして出力します。
  IMFSample* pSample = NULL;
  hr = ReadVideoFrame(pReader, &pSample);
  if (SUCCEEDED(hr))
  {
    if (pSample)
    {
      IMFMediaBuffer* buf = nullptr;
      hr = pSample->ConvertToContiguousBuffer(&buf);
      if (SUCCEEDED(hr))
      {
        byte* byteBuffer = nullptr;
        DWORD buffCurrLen = 0;
        hr = buf->Lock(&byteBuffer, nullptr, &buffCurrLen);
        if (SUCCEEDED(hr))
        {
          std::cout << "buffer current length : " << buffCurrLen << std::endl;
        }
        else
        {
          std::cerr << "Failed to Lock." << std::endl;
        }
      }
      else
      {
        std::cerr << "Failed to ConvertToContiguousBuffer." << std::endl;
      }
    }

    UINT32 width, height;
    hr = MFGetAttributeSize(pCurrentMediaType, MF_MT_FRAME_SIZE, &width, &height);
    if (SUCCEEDED(hr))
    {
      GUID subType = { 0 };
      hr = pCurrentMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
      if (SUCCEEDED(hr))
      {
        std::wstring formatName = GUIDToFormatName(subType);
        std::wcout << L"Video frame format: " << formatName << std::endl;
        if (subType == MFVideoFormat_RGB32)
        {
          std::wstring outputFilename = L"output.bmp";
          hr = SaveFrameAsBMP(pSample, outputFilename, width, height);
          if (SUCCEEDED(hr))
          {
            std::wcout << L"Saved frame as " << outputFilename << std::endl;
          }
          else
          {
            std::wcerr << L"Failed to save frame as " << outputFilename << std::endl;
          }
        }
        else
        {
          std::wcerr << L"Video frame format is not RGB32. Convert the format before saving as BMP." << std::endl;
        }
      }
    }
    pSample->Release();
  }
  else
  {
    std::cerr << "Failed to read video frame" << std::endl;

  }

  pCurrentMediaType->Release();

  // クリーンアップ
  if (pReader) pReader->Release();
  for (IMFMediaType* mediaType : mediaTypes) {
    mediaType->Release();
  }
  for (IMFActivate* device : devices) {
    device->Release();
  }

  MFShutdown();
  CoUninitialize();
  return 0;
}
