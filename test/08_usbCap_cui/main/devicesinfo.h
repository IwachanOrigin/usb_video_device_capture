
#ifndef DEVICES_INFO_H_
#define DEVICES_INFO_H_

#include "devicecommon.h"

#include <wrl/client.h>

#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>

using namespace Microsoft::WRL;

class DevicesInfo
{

public:
  explicit DevicesInfo();
  ~DevicesInfo();

  void setCurrentVideoDeviceIndex(const int index)
  {
    m_currentVideoDeviceIndex = index;
    this->getVideoDeviceMediaInfo();
  }
  void setCurrentAudioDeviceIndex(const int index)
  {
    m_currentAudioDeviceIndex = index;
    this->getAudioDeviceMediaInfo();
  }
  void setCurrentVideoFormatIndex(const int index)
  {
    m_currentVideoFormatIndex = index;
  }

  void writeDeviceNameList();
  void writeDeviceMediaInfoList();
  bool getVideoDeviceMediaInfo(const int index, DeviceMediaInfo& dmi);

private:
  int getDeviceNames();
  int getVideoDeviceMediaInfo();
  int getAudioDeviceMediaInfo();

  LPCSTR GetGUIDNameConst(const GUID& guid);
  HRESULT GetVideoSourceFromDevice(UINT nDevice, IMFMediaSource** ppVideoSource, IMFSourceReader** ppVideoReader);

  std::vector<DeviceInfo> m_devicesInfo;
  std::vector<DeviceMediaInfo> m_deviceMediaInfo;
  int m_currentVideoDeviceIndex;
  int m_currentAudioDeviceIndex;
  int m_currentVideoFormatIndex;

};

#endif // DEVICES_INFO_H_
