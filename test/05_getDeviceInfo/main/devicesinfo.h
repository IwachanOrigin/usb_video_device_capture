
#ifndef DEVICES_INFO_H_
#define DEVICES_INFO_H_

#include <vector>
#include <string>

class DevicesInfo
{
  struct DeviceInfo
  {
    std::wstring deviceName;
    std::wstring symbolicLink;
    std::wstring srcType;
  };

public:
  explicit DevicesInfo();
  ~DevicesInfo();

  void setCurrentVideoDeviceIndex(int index) { m_currentVideoDeviceIndex = index; }
  void setCurrentAudioDeviceIndex(int index) { m_currentAudioDeviceIndex = index; }

  void writeDeviceNameList();

private:
  int getDeviceNames();

  std::vector<DeviceInfo> devicesInfo;
  int m_currentVideoDeviceIndex;
  int m_currentAudioDeviceIndex;

};

#endif // DEVICES_INFO_H_
