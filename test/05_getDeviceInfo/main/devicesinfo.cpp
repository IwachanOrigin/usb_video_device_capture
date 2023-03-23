
#include "devicesinfo.h"

#include "MFUtility.h"

#include <iostream>
#include <string>
#include <memory>
#include <mfapi.h>
#include <mfidl.h>
#include <wrl/client.h>

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

int DevicesInfo::getDeviceMediaInfo()
{
  HRESULT hr = S_OK;

  ComPtr<IMFMediaSource> pVideoSource = nullptr;
  ComPtr<IMFSourceReader> pVideoReader = nullptr;

  CHECK_HR(GetVideoSourceFromDevice(m_currentVideoDeviceIndex, &pVideoSource, &pVideoReader), "Failed to get webcam video source.");

}