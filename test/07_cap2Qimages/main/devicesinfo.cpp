
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <cassert>

#include "devicesinfo.h"

DevicesInfo::DevicesInfo()
  : m_currentAudioDeviceIndex(0)
  , m_currentVideoDeviceIndex(0)
  , m_sampleCount(0)
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
    std::wcout << "Symbolic Link : " << m_devicesInfo[i].symbolicLink << std::endl;
    std::wcout << std::endl;
  }
}

void DevicesInfo::getDeviceNameList(std::vector<DeviceInfo>& vec)
{
  vec.clear();
  for (int i = 0; i < m_devicesInfo.size(); i++)
  {
    DeviceInfo di;
    di.deviceName = m_devicesInfo[i].deviceName;
    di.symbolicLink = m_devicesInfo[i].symbolicLink;
    vec.push_back(di);
  }
}

void DevicesInfo::writeDeviceMediaInfoList()
{
  for (int i = 0; i < m_deviceMediaInfo.size(); i++)
  {
    std::wcout << "No: " << i << std::endl;
    std::wcout << "formatSubtypeName : " << m_deviceMediaInfo[i].formatSubtypeName << std::endl;
    std::wcout << "width : " << m_deviceMediaInfo[i].width << std::endl;
    std::wcout << "height : " << m_deviceMediaInfo[i].height << std::endl;
    std::wcout << "interlaceMode : " << m_deviceMediaInfo[i].interlaceMode << std::endl;
    std::wcout << "stride : " << m_deviceMediaInfo[i].stride << std::endl;
    std::wcout << "aspect ratio : " << m_deviceMediaInfo[i].aspectRatioNumerator << " / " << m_deviceMediaInfo[i].aspectRatioDenominator << std::endl;
    std::wcout << "frame rate : " << m_deviceMediaInfo[i].frameRateNumerator << " / " << m_deviceMediaInfo[i].frameRateDenominator << std::endl;
    std::wcout << std::endl;
  }
}

void DevicesInfo::getVideoDeviceMediaList(std::vector<DeviceMediaInfo>& vec)
{
  vec.clear();
  for (int i = 0; i < m_deviceMediaInfo.size(); i++)
  {
    vec.push_back(m_deviceMediaInfo[i]);
  }
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

  CHECK_HR(GetVideoSourceFromDevice(m_currentVideoDeviceIndex, &pVideoSource, &pVideoReader), "Failed to get webcam video source.");
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
      pszGuidStr = GetGUIDNameConst(dmi.formatSubtypeGuid);
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

void DevicesInfo::captureStart()
{
  HRESULT hr = S_OK;

  m_finished = false;

  // Get the sources for the video and audio capture devices.
  hr = GetSourceFromCaptureDevice(DeviceType::Video, m_currentVideoDeviceIndex, &m_pVideoSource, &m_pSourceReader);
  if (FAILED(hr))
  {
    std::wcout << "Failed to get video source and reader." << std::endl;
    return;
  }

  hr = m_pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &m_pVideoSrcOutputType);
  if (FAILED(hr))
  {
    std::wcout << "Error retrieving current media type from first video stream." << std::endl;
    return;
  }

  hr = m_pSourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set the first video stream on the source reader." << std::endl;
    return;
  }

  hr = m_pVideoSource->CreatePresentationDescriptor(&m_pSrcPresentationDescriptor);
  if (FAILED(hr))
  {
    std::wcout << "Failed to create the presentation descriptor from the media source." << std::endl;
    return;
  }

  m_fSelected = false;
  hr = m_pSrcPresentationDescriptor->GetStreamDescriptorByIndex(0, &m_fSelected, &m_pSrcStreamDescriptor);
  if (FAILED(hr))
  {
    std::wcout << "Failed to get source stream descriptor from presentation descriptor." << std::endl;
    return;
  }

  //
  //
  //
  hr = MFCreateMediaType(&m_pVideoSrcOut);
  if (FAILED(hr))
  {
    std::wcout << "Failed to create media type." << std::endl;
    return;
  }

  hr = m_pVideoSrcOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set major video type." << std::endl;
    return;
  }

  hr = m_pVideoSrcOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set video sub-type attribute on media type." << std::endl;
    return;
  }

  hr = m_pVideoSrcOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set interlace mode attribute on media type." << std::endl;
    return;
  }

  hr = m_pVideoSrcOut->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set independent samples attribute on media type." << std::endl;
    return;
  }

  hr = MFSetAttributeRatio(m_pVideoSrcOut.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set pixel aspect ratio attribute on media type" << std::endl;
    return;
  }

  hr = MFSetAttributeSize(m_pVideoSrcOut.Get(), MF_MT_FRAME_SIZE, m_deviceMediaInfo[m_currentVideoFormatIndex].width, m_deviceMediaInfo[m_currentVideoFormatIndex].height);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set the frame size attribute on media type." << std::endl;
    return;
  }

  hr = MFSetAttributeSize(m_pVideoSrcOut.Get(), MF_MT_FRAME_RATE, m_deviceMediaInfo[m_currentVideoFormatIndex].frameRateNumerator, 1);
  if (FAILED(hr))
  {
    std::wcout << "Failed to set the frame rate attribute on media type." << std::endl;
    return;
  }

  hr = CopyAttribute(m_pVideoSrcOutputType.Get(), m_pVideoSrcOut.Get(), MF_MT_DEFAULT_STRIDE);
  if (FAILED(hr))
  {
    std::wcout << "Failed to copy default stride attribute." << std::endl;
    return;
  }

  hr = m_pSourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, m_pVideoSrcOut.Get());
  if (FAILED(hr))
  {
    std::wcout << "Failed to set video media type on source reader." << std::endl;
    return;
  }

  DWORD stmIndex = 0;
  BOOL isSelected = false;
  DWORD srcVideoStreamIndex = 0;
  while (m_pSourceReader->GetStreamSelection(stmIndex, &isSelected) == S_OK)
  {
    printf("Stream %d is selected %d.\n", stmIndex, isSelected);

    hr = m_pSourceReader->GetCurrentMediaType(stmIndex, &m_pStreamMediaType);
    if (FAILED(hr))
    {
      std::wcout << "Failed to get media type for selected stream." << std::endl;
    }
    std::cout << "Media type: " << GetMediaTypeDescription(m_pStreamMediaType.Get()) << std::endl;

    GUID majorMediaType;
    m_pStreamMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorMediaType);
    if (majorMediaType == MFMediaType_Video)
    {
      std::cout << "Source video stream index is " << stmIndex << "." << std::endl;
      srcVideoStreamIndex = stmIndex;
    }
    stmIndex++;
  }

  this->updateImage();
}

void DevicesInfo::captureStop()
{
  m_finished = true;
}

void DevicesInfo::updateImage()
{
  if (m_finished)
  {
    return;
  }
  assert(m_pSourceReader);

  DWORD streamIndex = 0;
  DWORD flags = 0;
  LONGLONG llSampleTimeStamp = 0;
  ComPtr<IMFSample> pSample = NULL;
  LONGLONG llVideoBaseTime = 0;

  HRESULT hr;
  hr = m_pSourceReader->ReadSample(
    MF_SOURCE_READER_ANY_STREAM,
    0,																	// Flags.
    &streamIndex,												// Receives the actual stream index. 
    &flags,															// Receives status flags.
    &llSampleTimeStamp,									// Receives the time stamp.
    &pSample												// Receives the sample or NULL.
  );

  if (hr != S_OK)
  {
    m_finished = true;
    return;
  }

  if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
  {
    printf("End of stream.\n");
  }
  if (flags & MF_SOURCE_READERF_STREAMTICK)
  {
    printf("Stream tick.\n");
  }

  if (pSample)
  {
    hr = pSample->SetSampleTime(llSampleTimeStamp);
    assert(hr == S_OK);

    ComPtr<IMFMediaBuffer> buf = nullptr;
    DWORD bufLength = 0;

    hr = pSample->ConvertToContiguousBuffer(&buf);
    hr = buf->GetCurrentLength(&bufLength);

    byte* byteBuffer = NULL;
    DWORD buffMaxLen = 0, buffCurrLen = 0;
    hr = buf->Lock(&byteBuffer, &buffMaxLen, &buffCurrLen);
    assert(hr == S_OK);

    buf->Unlock();
    SAFE_RELEASE(buf);
    m_sampleCount++;
  }
  SAFE_RELEASE(pSample);
}
