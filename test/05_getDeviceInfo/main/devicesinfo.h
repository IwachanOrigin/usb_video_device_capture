
#ifndef DEVICES_INFO_H_
#define DEVICES_INFO_H_

#include <vector>
#include <string>

#include <mfapi.h>

class DevicesInfo
{
  struct DeviceInfo
  {
    std::wstring deviceName;
    std::wstring symbolicLink;
  };

  struct DeviceMediaInfo
  {
    int width;
    int height;
    int stride;
    int samplesize;
    GUID formatSubtype;
    std::wstring formatSubtypeName;
  };

public:
  explicit DevicesInfo();
  ~DevicesInfo();

  void setCurrentVideoDeviceIndex(int index) { m_currentVideoDeviceIndex = index; }
  void setCurrentAudioDeviceIndex(int index) { m_currentAudioDeviceIndex = index; }

  void writeDeviceNameList();

private:
  int getDeviceNames();
  int getDeviceMediaInfo();

  std::vector<DeviceInfo> m_devicesInfo;
  std::vector<DeviceMediaInfo> m_deviceMediaInfo;
  int m_currentVideoDeviceIndex;
  int m_currentAudioDeviceIndex;

};

#endif // DEVICES_INFO_H_
