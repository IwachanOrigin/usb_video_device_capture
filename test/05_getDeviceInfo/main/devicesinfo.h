
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
    uint32_t width;
    uint32_t height;
    uint32_t interlaceMode;
    LONG stride;
    uint32_t aspectRatioNumerator;
    uint32_t aspectRatioDenominator;
    uint32_t frameRateNumerator;
    uint32_t frameRateDenominator;
    GUID formatSubtypeGuid;
    std::wstring formatSubtypeName;

    // for std::unique
    bool operator==(const DeviceMediaInfo& dmi)
    {
      if (dmi.width == this->width
        && dmi.height == this->height
        && dmi.interlaceMode == this->interlaceMode
        && dmi.stride == this->stride
        && dmi.aspectRatioNumerator == this->aspectRatioNumerator
        && dmi.aspectRatioDenominator == this->aspectRatioDenominator
        && dmi.frameRateNumerator == this->frameRateNumerator
        && dmi.frameRateDenominator == this->frameRateDenominator
        && dmi.formatSubtypeGuid == this->formatSubtypeGuid)
      {
        return true;
      }
      return false;
    }

    // for std::sort
    bool operator<(const DeviceMediaInfo& dmi) const
    {
      if (this->width < dmi.width
        && this->height < dmi.height
        && this->interlaceMode < dmi.interlaceMode
        && this->stride < dmi.stride
        && this->aspectRatioNumerator < dmi.aspectRatioNumerator
        && this->aspectRatioDenominator < dmi.aspectRatioDenominator
        && this->frameRateNumerator < dmi.frameRateNumerator
        && this->frameRateDenominator < dmi.frameRateDenominator
        && this->formatSubtypeName < dmi.formatSubtypeName)
      {
        return true;
      }
      return false;
    }
  };

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
