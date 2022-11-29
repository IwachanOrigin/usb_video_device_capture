
#ifndef UTILS_H_
#define UTILS_H_

#include "stdafx.h"
#include "dxhelper.h"

namespace helper
{

HRESULT utilCopyAttribute(IMFAttributes* srcAttribute, IMFAttributes* dstAttribute, const GUID& key)
{
  PROPVARIANT var;
  PropVariantInit(&var);
  HRESULT hr = srcAttribute->GetItem(key, &var);
  if (SUCCEEDED(hr))
  {
    hr = dstAttribute->SetItem(key, var);
    PropVariantClear(&var);
  }
  return hr;
}

// Create a compatible video format with a different subtype.
HRESULT utilCloneVideomediaType(IMFMediaType* srcMediaType, REFGUID guidSubType, IMFMediaType** ppNewMediaType)
{
  IMFMediaType* newMediaType = nullptr;

  HRESULT hr = MFCreateMediaType(&newMediaType);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = newMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = newMediaType->SetGUID(MF_MT_MAJOR_TYPE, guidSubType);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType, MF_MT_FRAME_SIZE);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType, MF_MT_FRAME_RATE);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType, MF_MT_PIXEL_ASPECT_RATIO);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType, MF_MT_INTERLACE_MODE);
  if (FAILED(hr))
  {
    goto Exit;
  }

  *ppNewMediaType = newMediaType;
  (*ppNewMediaType)->AddRef();

Exit:
  SAFE_RELEASE(newMediaType);

  return hr;
}

// Helper function to get the frame size from a video media type.
inline HRESULT utilGetFrameSize(IMFMediaType* type, UINT32* width, UINT32* height)
{
  return MFGetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
}

// Helper function to get the frame rate from a video media type.
inline HRESULT utilGetFrameRate(IMFMediaType* type, UINT32* numerator, UINT32* denominator)
{
  return MFGetAttributeRatio(type, MF_MT_FRAME_RATE, numerator, denominator);
}

HRESULT utilGetEncodingBitrate(IMFMediaType* mediaType, UINT32* encodingBitrate)
{
  UINT32 width = 0;
  UINT32 height = 0;
  float bitrate = 0.0f;
  UINT32 framerateNum = 0;
  UINT32 framerateDenom = 0;

  HRESULT hr = utilGetFrameSize(mediaType, &width, &height);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilGetFrameRate(mediaType, &framerateNum, &framerateDenom);
  if (FAILED(hr))
  {
    return hr;
  }

  bitrate = width / 3.0f * height * framerateNum / framerateDenom;
  *encodingBitrate = static_cast<UINT32>(bitrate);

  return hr;
}

HRESULT utilConfigureVideoEncodeing(IMFCaptureSource* capSrc, IMFCaptureRecordSink* capRecord, REFGUID guidEncodingType)
{
  IMFMediaType* mediatype = nullptr;
  IMFMediaType* mediatype2 = nullptr;
  GUID guidSubType = GUID_NULL;

  // Configure the video format for the recording sink.
  HRESULT hr = capSrc->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, &mediatype);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = utilCloneVideomediaType(mediatype, guidEncodingType, &mediatype2);
  if (FAILED(hr))
  {
    goto Exit;
  }

  hr = mediatype->GetGUID(MF_MT_SUBTYPE, &guidSubType);
  if (FAILED(hr))
  {
    goto Exit;
  }

  if (guidSubType == MFVideoFormat_H264_ES || guidSubType == MFVideoFormat_H264)
  {
    // When the webcam supports H264_ES or H264, we just bypass the stream.
    // The output from capture engine shall be the same as the native type supported by the webcam.
    hr = mediatype2->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
  }
  else
  {
    UINT32 encodingBitrate = 0;
    hr = utilGetEncodingBitrate(mediatype2, &encodingBitrate);
  }

Exit:
  SAFE_RELEASE(mediatype);
  SAFE_RELEASE(mediatype2);

  return hr;
}

} // helper

#endif // UTILS_H_