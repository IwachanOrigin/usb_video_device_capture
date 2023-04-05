
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
  void getDeviceNameList(std::vector<DeviceInfo>& vec);
  void getVideoDeviceMediaList(std::vector<DeviceMediaInfo>& vec);

  void captureStart();
  void captureStop();
  void updateImage(unsigned char* buffer);

private:
  int getDeviceNames();
  int getVideoDeviceMediaInfo();
  int getAudioDeviceMediaInfo();

  std::vector<DeviceInfo> m_devicesInfo;
  std::vector<DeviceMediaInfo> m_deviceMediaInfo;
  int m_currentVideoDeviceIndex;
  int m_currentAudioDeviceIndex;
  int m_currentVideoFormatIndex;

  ComPtr<IMFMediaSource> m_pVideoSource;
  ComPtr<IMFSourceReader> m_pSourceReader;
  ComPtr<IMFMediaType> m_pVideoSrcOutputType;
  ComPtr<IMFPresentationDescriptor> m_pSrcPresentationDescriptor;
  ComPtr<IMFStreamDescriptor> m_pSrcStreamDescriptor;
  BOOL m_fSelected;
  ComPtr<IMFMediaType> m_pVideoSrcOut;
  ComPtr<IMFMediaType> m_pStreamMediaType;
  ComPtr<IMFSample> m_pSample;

  bool m_finished;
  int m_sampleCount;

};

#endif // DEVICES_INFO_H_
