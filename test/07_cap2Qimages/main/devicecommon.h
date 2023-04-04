
#ifndef DEVICE_COMMON_H_
#define DEVICE_COMMON_H_

#include <vector>
#include <string>

#include <mfapi.h>

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
      if (this->formatSubtypeName < dmi.formatSubtypeName)
      {
        return true;
      }
      return false;
    }
};

#endif // DEVICE_COMMON_H_
