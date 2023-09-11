
#ifndef VIDEO_CAPTURE_FORMAT_H_
#define VIDEO_CAPTURE_FORMAT_H_

enum class VideoCaptureFormat
{
    VideoCapFmt_NONE  = -1
  , VideoCapFmt_NV12  = 0
  , VideoCapFmt_YUY2  = 1
  , VideoCapFmt_RGB32 = 2
  , VideoCapFmt_DMO   = 3
};

#endif // VIDEO_CAPTURE_FORMAT_H_
