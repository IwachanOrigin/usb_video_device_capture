
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>

#include <mfapi.h>
#include <mfidl.h>
#include <wrl/client.h>

#include "MFUtility.h"

#include "devicesinfo.h"

using namespace Microsoft::WRL;

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
    std::wcout << "Symbolic Link : " << m_devicesInfo[i].symbolicLink << std::endl;
    std::wcout << std::endl;
  }
}

void DevicesInfo::writeDeviceMediaInfoList()
{
  for (int i = 0; i < m_deviceMediaInfo.size(); i++)
  {
    std::wcout << L"No: " << i << std::endl;
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