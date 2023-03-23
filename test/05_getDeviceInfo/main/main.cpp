
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <mfapi.h>
#include <mfidl.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mf.lib")

HRESULT getVideoDeviceNames(std::vector<std::wstring>& deviceNames)
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

  deviceNames.resize(count);
  for (int i = 0; i < count; i++)
  {
    wchar_t* buffer = nullptr;
    uint32_t length = 0;
    hr = ppRawDevice[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &buffer, &length);
    if (FAILED(hr))
    {
      return hr;
    }
    deviceNames[i] = buffer;
  }

  if (ppRawDevice != nullptr)
  {
    for (int i = 0; i < count; i++)
    {
      ppRawDevice[i]->Release();
    }
    CoTaskMemFree(ppRawDevice);
  }

  return S_OK;
}

int main(int argc, char* argv[])
{
  HRESULT hr;
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    return hr;
  }

  std::vector<std::wstring> deviceNames;
  hr = getVideoDeviceNames(deviceNames);
  if (FAILED(hr))
  {
    return hr;
  }

  for (const auto& wstr : deviceNames)
  {
    std::wcout << wstr << std::endl;
  }

  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  return 0;
}
