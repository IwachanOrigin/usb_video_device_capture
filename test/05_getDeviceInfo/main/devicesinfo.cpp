
#include "devicesinfo.h"

#include <iostream>
#include <string>
#include <memory>
#include <mfapi.h>
#include <mfidl.h>

DevicesInfo::DevicesInfo()
{
  this->getDeviceNames();
}

DevicesInfo::~DevicesInfo()
{
}

void DevicesInfo::writeDeviceNameList()
{
  for (int i = 0; i < devicesInfo.size(); i++)
  {
    std::wcout << L"No: " << i << " Name : " << devicesInfo[i].deviceName;
  }
}

int DevicesInfo::getDeviceNames()
{
  HRESULT hr;

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

  devicesInfo.resize(count);
  for (uint32_t i = 0; i < count; i++)
  {
    wchar_t* buffer = nullptr;
    uint32_t length = 0;
    hr = ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length);
    if (FAILED(hr))
    {
      return hr;
    }
    devicesInfo[i].deviceName = buffer;
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
