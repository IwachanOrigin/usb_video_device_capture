
#ifndef DEVICES_INFO_H_
#define DEVICES_INFO_H_

#include "devicecommon.h"

class DevicesInfo
{

public:
  explicit DevicesInfo();
  ~DevicesInfo();

  void setCurrentVideoDeviceIndex(int index)
  {
    m_currentVideoDeviceIndex = index;
    this->getVideoDeviceMediaInfo();
  }
  void setCurrentAudioDeviceIndex(int index)
  {
    m_currentAudioDeviceIndex = index;
    this->getAudioDeviceMediaInfo();
  }

  void writeDeviceNameList();
  void writeDeviceMediaInfoList();
  void getDeviceNameList(std::vector<DeviceInfo>& vec);
  void getVideoDeviceMediaList(std::vector<DeviceMediaInfo>& vec);

private:
  int getDeviceNames();
  int getVideoDeviceMediaInfo();
  int getAudioDeviceMediaInfo();

  std::vector<DeviceInfo> m_devicesInfo;
  std::vector<DeviceMediaInfo> m_deviceMediaInfo;
  int m_currentVideoDeviceIndex;
  int m_currentAudioDeviceIndex;

};

#endif // DEVICES_INFO_H_
