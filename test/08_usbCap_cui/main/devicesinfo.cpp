
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <cassert>

#include "devicesinfo.h"

#define CHECK_HR(hr, msg) if (hr != S_OK) { printf(msg); printf(" Error: %.2X.\n", hr);}
#define CHECKHR_GOTO(x, y) if(FAILED(x)) goto y
#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return #val
#endif

template <class T> void SAFE_RELEASE(T * *ppT)
{
  if (*ppT)
  {
    (*ppT)->Release();
    *ppT = nullptr;
  }
}

template <class T> inline void SAFE_RELEASE(T * &pT)
{
  if (pT != nullptr)
  {
    pT->Release();
    pT = nullptr;
  }
}

template <class T> void SAFE_RELEASE(T p)
{
  if (p)
  {
    (p)->Release();
    p = nullptr;
  }
}

DevicesInfo::DevicesInfo()
  : m_currentAudioDeviceIndex(0)
  , m_currentVideoDeviceIndex(0)
{
  this->getDeviceNames();
}

DevicesInfo::~DevicesInfo()
{
}

void DevicesInfo::writeDeviceNameList()
{
  for (int i = 0; i < m_devicesInfo.size(); i++)
  {
    std::wcout << L"No: " << i << std::endl;
    std::wcout << "Name : " << m_devicesInfo[i].deviceName << std::endl;
    std::wcout << std::endl;
  }
}

void DevicesInfo::writeDeviceMediaInfoList()
{
  for (int i = 0; i < m_deviceMediaInfo.size(); i++)
  {
    OLECHAR* guidString = nullptr;
    HRESULT hr = StringFromCLSID(m_deviceMediaInfo[i].formatSubtypeGuid, &guidString);
    std::wcout << "No: " << i
               << ", " << m_deviceMediaInfo[i].width << " x " << m_deviceMediaInfo[i].height
               << ", " << guidString
               << ", " << m_deviceMediaInfo[i].formatSubtypeName
               << ", " << m_deviceMediaInfo[i].stride
               << ", " << m_deviceMediaInfo[i].frameRateNumerator / m_deviceMediaInfo[i].frameRateDenominator
               << ", " << m_deviceMediaInfo[i].samplesize
               << std::endl;
    ::CoTaskMemFree(guidString);
  }
}

bool DevicesInfo::getVideoDeviceMediaInfo(const int index, DeviceMediaInfo& dmi)
{
  if (index < 0 || index >= m_deviceMediaInfo.size())
  {
    return false;
  }

  dmi = m_deviceMediaInfo[index];

  return true;
}

int DevicesInfo::getDeviceNames()
{
  HRESULT hr = S_OK;;

  std::shared_ptr<IMFAttributes> pAttributes;
  IMFAttributes *pRawAttributes = nullptr;
  hr = MFCreateAttributes(&pRawAttributes, 1);
  if (FAILED(hr))
  {
    return hr;
  }
  pAttributes = std::shared_ptr<IMFAttributes>(pRawAttributes, [](auto* p)
    { p->Release(); });

  hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr))
  {
    return hr;
  }

  IMFActivate** ppRawDevice = nullptr;
  uint32_t count = 0;
  hr = MFEnumDeviceSources(pAttributes.get(), &ppRawDevice, &count);
  if (FAILED(hr))
  {
    return hr;
  }

  m_devicesInfo.clear();
  m_devicesInfo.resize(count);
  for (uint32_t i = 0; i < count; i++)
  {
    wchar_t* name = nullptr;
    uint32_t length = 0;
    hr = ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &length);
    if (FAILED(hr))
    {
      std::wcout << "Failed to get the device name. index = " << i << std::endl;
      continue;
    }
    m_devicesInfo[i].deviceName = name;

    wchar_t* symlink = nullptr;
    hr = ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symlink, &length);
    if (FAILED(hr))
    {
      std::wcout << "Failed to get the symbolic link. index = " << i << std::endl;
      continue;
    }
    m_devicesInfo[i].symbolicLink = symlink;
  }

  if (ppRawDevice != nullptr)
  {
    for (uint32_t i = 0; i < count; i++)
    {
      ppRawDevice[i]->Release();
    }
    CoTaskMemFree(ppRawDevice);
  }

  return S_OK;
}

int DevicesInfo::getVideoDeviceMediaInfo()
{
  HRESULT hr = S_OK;
  ComPtr<IMFMediaSource> pVideoSource = nullptr;
  ComPtr<IMFSourceReader> pVideoReader = nullptr;
  ComPtr<IMFPresentationDescriptor> pSourcePresentationDescriptor = nullptr;
  BOOL fSelected = false;

  m_deviceMediaInfo.clear();

  CHECK_HR(getVideoSourceFromDevice(m_currentVideoDeviceIndex, &pVideoSource, &pVideoReader), "Failed to get webcam video source.");
  CHECK_HR(pVideoSource->CreatePresentationDescriptor(&pSourcePresentationDescriptor), "Failed to create the presentation descriptor from the media source.");
  
  DWORD streamDescCount = 0;
  CHECK_HR(pSourcePresentationDescriptor->GetStreamDescriptorCount(&streamDescCount), "Failed to get stream descriptor count.");

  for (int descIndex = 0; descIndex < (int)streamDescCount; descIndex++)
  {
    ComPtr<IMFStreamDescriptor> pSourceStreamDescriptor = nullptr;
    CHECK_HR(pSourcePresentationDescriptor->GetStreamDescriptorByIndex(descIndex, &fSelected, &pSourceStreamDescriptor), "Failed to get source stream descriptor from presentation descriptor");
    
    ComPtr<IMFMediaTypeHandler> pSourceMediaTypeHandler = nullptr;
    CHECK_HR(pSourceStreamDescriptor->GetMediaTypeHandler(&pSourceMediaTypeHandler), "Failed to get source media type handler.");

    DWORD typeCount = 0;
    CHECK_HR(pSourceMediaTypeHandler->GetMediaTypeCount(&typeCount), "Failed to get source media type count.");

    for (int typeIndex = 0; typeIndex < (int)typeCount; typeIndex++)
    {
      ComPtr<IMFMediaType> pMediaType = nullptr;
      CHECK_HR(pSourceMediaTypeHandler->GetMediaTypeByIndex(typeIndex, &pMediaType), "Error retrieving media type.");
      DeviceMediaInfo dmi{};
      CHECK_HR(MFGetAttributeSize(pMediaType.Get(), MF_MT_FRAME_SIZE, &dmi.width, &dmi.height), "Failed to get the frame size attribute on media type.");
      CHECK_HR(pMediaType->GetGUID(MF_MT_SUBTYPE, &dmi.formatSubtypeGuid), "Failed to get the subtype guid on media type.");
      CHECK_HR(pMediaType->GetUINT32(MF_MT_INTERLACE_MODE, &dmi.interlaceMode), "Failed to get the interlace mode on media type.");
      CHECK_HR(pMediaType->GetUINT32(MF_MT_SAMPLE_SIZE, &dmi.samplesize), "Failed to get the sample size on media type.");
      LONG unDefault = 1;
      dmi.stride = (LONG)MFGetAttributeUINT32(pMediaType.Get(), MF_MT_DEFAULT_STRIDE, unDefault);
      CHECK_HR(MFGetAttributeRatio(pMediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, &dmi.aspectRatioNumerator, &dmi.aspectRatioDenominator), "Failed to get the pixel aspect ratio on media type.");
      CHECK_HR(MFGetAttributeRatio(pMediaType.Get(), MF_MT_FRAME_RATE, &dmi.frameRateNumerator, &dmi.frameRateDenominator), "Failed to get the frame rate on media type.");

      LPCSTR pszGuidStr = "";
      pszGuidStr = getGUIDNameConst(dmi.formatSubtypeGuid);
      if (pszGuidStr != nullptr)
      {
        std::string tempStr = pszGuidStr;
        dmi.formatSubtypeName = std::wstring(tempStr.begin(), tempStr.end());
      }
      else
      {
        LPOLESTR guidStr = nullptr;

        CHECKHR_GOTO(StringFromCLSID(dmi.formatSubtypeGuid, &guidStr), done);
        dmi.formatSubtypeName = std::wstring(guidStr);
        CoTaskMemFree(guidStr);
      }
      m_deviceMediaInfo.push_back(dmi);
    }
  }

  // Remove duplicates
  std::sort(m_deviceMediaInfo.begin(), m_deviceMediaInfo.end());
  m_deviceMediaInfo.erase(std::unique(m_deviceMediaInfo.begin(), m_deviceMediaInfo.end()), m_deviceMediaInfo.end());

done:

  std::wcout << "finished." << std::endl;

  return hr;
}

int DevicesInfo::getAudioDeviceMediaInfo()
{
  HRESULT hr = S_OK;
  return hr;
}

LPCSTR DevicesInfo::getGUIDNameConst(const GUID& guid)
{
  IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
  IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
  IF_EQUAL_RETURN(guid, MF_MT_SUBTYPE);
  IF_EQUAL_RETURN(guid, MF_MT_ALL_SAMPLES_INDEPENDENT);
  IF_EQUAL_RETURN(guid, MF_MT_FIXED_SIZE_SAMPLES);
  IF_EQUAL_RETURN(guid, MF_MT_COMPRESSED);
  IF_EQUAL_RETURN(guid, MF_MT_SAMPLE_SIZE);
  IF_EQUAL_RETURN(guid, MF_MT_WRAPPED_TYPE);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_NUM_CHANNELS);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_SECOND);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BLOCK_ALIGNMENT);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BITS_PER_SAMPLE);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_BLOCK);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_CHANNEL_MASK);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FOLDDOWN_MATRIX);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKREF);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKTARGET);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGREF);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGTARGET);
  IF_EQUAL_RETURN(guid, MF_MT_AUDIO_PREFER_WAVEFORMATEX);
  IF_EQUAL_RETURN(guid, MF_MT_AAC_PAYLOAD_TYPE);
  IF_EQUAL_RETURN(guid, MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
  IF_EQUAL_RETURN(guid, MF_MT_FRAME_SIZE);
  IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE);
  IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MAX);
  IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MIN);
  IF_EQUAL_RETURN(guid, MF_MT_PIXEL_ASPECT_RATIO);
  IF_EQUAL_RETURN(guid, MF_MT_DRM_FLAGS);
  IF_EQUAL_RETURN(guid, MF_MT_PAD_CONTROL_FLAGS);
  IF_EQUAL_RETURN(guid, MF_MT_SOURCE_CONTENT_HINT);
  IF_EQUAL_RETURN(guid, MF_MT_VIDEO_CHROMA_SITING);
  IF_EQUAL_RETURN(guid, MF_MT_INTERLACE_MODE);
  IF_EQUAL_RETURN(guid, MF_MT_TRANSFER_FUNCTION);
  IF_EQUAL_RETURN(guid, MF_MT_VIDEO_PRIMARIES);
  IF_EQUAL_RETURN(guid, MF_MT_CUSTOM_VIDEO_PRIMARIES);
  IF_EQUAL_RETURN(guid, MF_MT_YUV_MATRIX);
  IF_EQUAL_RETURN(guid, MF_MT_VIDEO_LIGHTING);
  IF_EQUAL_RETURN(guid, MF_MT_VIDEO_NOMINAL_RANGE);
  IF_EQUAL_RETURN(guid, MF_MT_GEOMETRIC_APERTURE);
  IF_EQUAL_RETURN(guid, MF_MT_MINIMUM_DISPLAY_APERTURE);
  IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_APERTURE);
  IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_ENABLED);
  IF_EQUAL_RETURN(guid, MF_MT_AVG_BITRATE);
  IF_EQUAL_RETURN(guid, MF_MT_AVG_BIT_ERROR_RATE);
  IF_EQUAL_RETURN(guid, MF_MT_MAX_KEYFRAME_SPACING);
  IF_EQUAL_RETURN(guid, MF_MT_DEFAULT_STRIDE);
  IF_EQUAL_RETURN(guid, MF_MT_PALETTE);
  IF_EQUAL_RETURN(guid, MF_MT_USER_DATA);
  IF_EQUAL_RETURN(guid, MF_MT_AM_FORMAT_TYPE);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG_START_TIME_CODE);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG2_PROFILE);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG2_LEVEL);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG2_FLAGS);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG_SEQUENCE_HEADER);
  IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_0);
  IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_0);
  IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_1);
  IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_1);
  IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_SRC_PACK);
  IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_CTRL_PACK);
  IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_HEADER);
  IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_FORMAT);
  IF_EQUAL_RETURN(guid, MF_MT_IMAGE_LOSS_TOLERANT);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG4_SAMPLE_DESCRIPTION);
  IF_EQUAL_RETURN(guid, MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
  IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_4CC);
  IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_WAVE_FORMAT_TAG);

  // Media types

  IF_EQUAL_RETURN(guid, MFMediaType_Audio);
  IF_EQUAL_RETURN(guid, MFMediaType_Video);
  IF_EQUAL_RETURN(guid, MFMediaType_Protected);
  IF_EQUAL_RETURN(guid, MFMediaType_SAMI);
  IF_EQUAL_RETURN(guid, MFMediaType_Script);
  IF_EQUAL_RETURN(guid, MFMediaType_Image);
  IF_EQUAL_RETURN(guid, MFMediaType_HTML);
  IF_EQUAL_RETURN(guid, MFMediaType_Binary);
  IF_EQUAL_RETURN(guid, MFMediaType_FileTransfer);

  IF_EQUAL_RETURN(guid, MFVideoFormat_AI44); //     FCC('AI44')
  IF_EQUAL_RETURN(guid, MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8 
  IF_EQUAL_RETURN(guid, MFVideoFormat_AYUV); //     FCC('AYUV')
  IF_EQUAL_RETURN(guid, MFVideoFormat_DV25); //     FCC('dv25')
  IF_EQUAL_RETURN(guid, MFVideoFormat_DV50); //     FCC('dv50')
  IF_EQUAL_RETURN(guid, MFVideoFormat_DVH1); //     FCC('dvh1')
  IF_EQUAL_RETURN(guid, MFVideoFormat_DVSD); //     FCC('dvsd')
  IF_EQUAL_RETURN(guid, MFVideoFormat_DVSL); //     FCC('dvsl')
  IF_EQUAL_RETURN(guid, MFVideoFormat_H264); //     FCC('H264')
  IF_EQUAL_RETURN(guid, MFVideoFormat_I420); //     FCC('I420')
  IF_EQUAL_RETURN(guid, MFVideoFormat_IYUV); //     FCC('IYUV')
  IF_EQUAL_RETURN(guid, MFVideoFormat_M4S2); //     FCC('M4S2')
  IF_EQUAL_RETURN(guid, MFVideoFormat_MJPG);
  IF_EQUAL_RETURN(guid, MFVideoFormat_MP43); //     FCC('MP43')
  IF_EQUAL_RETURN(guid, MFVideoFormat_MP4S); //     FCC('MP4S')
  IF_EQUAL_RETURN(guid, MFVideoFormat_MP4V); //     FCC('MP4V')
  IF_EQUAL_RETURN(guid, MFVideoFormat_MPG1); //     FCC('MPG1')
  IF_EQUAL_RETURN(guid, MFVideoFormat_MSS1); //     FCC('MSS1')
  IF_EQUAL_RETURN(guid, MFVideoFormat_MSS2); //     FCC('MSS2')
  IF_EQUAL_RETURN(guid, MFVideoFormat_NV11); //     FCC('NV11')
  IF_EQUAL_RETURN(guid, MFVideoFormat_NV12); //     FCC('NV12')
  IF_EQUAL_RETURN(guid, MFVideoFormat_P010); //     FCC('P010')
  IF_EQUAL_RETURN(guid, MFVideoFormat_P016); //     FCC('P016')
  IF_EQUAL_RETURN(guid, MFVideoFormat_P210); //     FCC('P210')
  IF_EQUAL_RETURN(guid, MFVideoFormat_P216); //     FCC('P216')
  IF_EQUAL_RETURN(guid, MFVideoFormat_RGB24); //    D3DFMT_R8G8B8 
  IF_EQUAL_RETURN(guid, MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8 
  IF_EQUAL_RETURN(guid, MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5 
  IF_EQUAL_RETURN(guid, MFVideoFormat_RGB565); //   D3DFMT_R5G6B5 
  IF_EQUAL_RETURN(guid, MFVideoFormat_RGB8);
  IF_EQUAL_RETURN(guid, MFVideoFormat_UYVY); //     FCC('UYVY')
  IF_EQUAL_RETURN(guid, MFVideoFormat_v210); //     FCC('v210')
  IF_EQUAL_RETURN(guid, MFVideoFormat_v410); //     FCC('v410')
  IF_EQUAL_RETURN(guid, MFVideoFormat_WMV1); //     FCC('WMV1')
  IF_EQUAL_RETURN(guid, MFVideoFormat_WMV2); //     FCC('WMV2')
  IF_EQUAL_RETURN(guid, MFVideoFormat_WMV3); //     FCC('WMV3')
  IF_EQUAL_RETURN(guid, MFVideoFormat_WVC1); //     FCC('WVC1')
  IF_EQUAL_RETURN(guid, MFVideoFormat_Y210); //     FCC('Y210')
  IF_EQUAL_RETURN(guid, MFVideoFormat_Y216); //     FCC('Y216')
  IF_EQUAL_RETURN(guid, MFVideoFormat_Y410); //     FCC('Y410')
  IF_EQUAL_RETURN(guid, MFVideoFormat_Y416); //     FCC('Y416')
  IF_EQUAL_RETURN(guid, MFVideoFormat_Y41P);
  IF_EQUAL_RETURN(guid, MFVideoFormat_Y41T);
  IF_EQUAL_RETURN(guid, MFVideoFormat_YUY2); //     FCC('YUY2')
  IF_EQUAL_RETURN(guid, MFVideoFormat_YV12); //     FCC('YV12')
  IF_EQUAL_RETURN(guid, MFVideoFormat_YVYU);

  IF_EQUAL_RETURN(guid, MFAudioFormat_PCM); //              WAVE_FORMAT_PCM 
  IF_EQUAL_RETURN(guid, MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT 
  IF_EQUAL_RETURN(guid, MFAudioFormat_DTS); //              WAVE_FORMAT_DTS 
  IF_EQUAL_RETURN(guid, MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF 
  IF_EQUAL_RETURN(guid, MFAudioFormat_DRM); //              WAVE_FORMAT_DRM 
  IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2 
  IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3 
  IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS 
  IF_EQUAL_RETURN(guid, MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF 
  IF_EQUAL_RETURN(guid, MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9 
  IF_EQUAL_RETURN(guid, MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3 
  IF_EQUAL_RETURN(guid, MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG 
  IF_EQUAL_RETURN(guid, MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC 
  IF_EQUAL_RETURN(guid, MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC 

  return NULL;
}


/**
* Gets a video source reader from a device such as a webcam.
* @param[in] nDevice: the video device index to attempt to get the source reader for.
* @param[out] ppVideoSource: will be set with the source for the reader if successful.
* @param[out] ppVideoReader: will be set with the reader if successful. Set this parameter
*  to nullptr if no reader is required and only the source is needed.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT DevicesInfo::getVideoSourceFromDevice(UINT nDevice, IMFMediaSource** ppVideoSource, IMFSourceReader** ppVideoReader)
{
  UINT32 videoDeviceCount = 0;
  IMFAttributes* videoConfig = NULL;
  IMFActivate** videoDevices = NULL;
  WCHAR* webcamFriendlyName;
  UINT nameLength = 0;
  IMFAttributes* pAttributes = NULL;

  HRESULT hr = S_OK;

  // Get the first available webcam.
  hr = MFCreateAttributes(&videoConfig, 1);
  CHECK_HR(hr, "Error creating video configuration.");

  // Request video capture devices.
  hr = videoConfig->SetGUID(
    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  CHECK_HR(hr, "Error initialising video configuration object.");

  hr = MFEnumDeviceSources(videoConfig, &videoDevices, &videoDeviceCount);
  CHECK_HR(hr, "Error enumerating video devices.");

  if (nDevice >= videoDeviceCount) {
    printf("The device index of %d was invalid for available device count of %d.\n", nDevice, videoDeviceCount);
    hr = E_INVALIDARG;
  }
  else {
    hr = videoDevices[nDevice]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &webcamFriendlyName, &nameLength);
    CHECK_HR(hr, "Error retrieving video device friendly name.\n");

    wprintf(L"Using webcam: %s\n", webcamFriendlyName);

    hr = videoDevices[nDevice]->ActivateObject(IID_PPV_ARGS(ppVideoSource));
    CHECK_HR(hr, "Error activating video device.");

    CHECK_HR(MFCreateAttributes(&pAttributes, 1),
      "Failed to create attributes.");

    if (ppVideoReader != nullptr) {
      // Adding this attribute creates a video source reader that will handle
      // colour conversion and avoid the need to manually convert between RGB24 and RGB32 etc.
      CHECK_HR(pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1),
        "Failed to set enable video processing attribute.");

      // Create a source reader.
      hr = MFCreateSourceReaderFromMediaSource(
        *ppVideoSource,
        pAttributes,
        ppVideoReader);
      CHECK_HR(hr, "Error creating video source reader.");
    }
  }

  SAFE_RELEASE(videoConfig);
  SAFE_RELEASE(videoDevices);
  SAFE_RELEASE(pAttributes);

  return hr;
}
