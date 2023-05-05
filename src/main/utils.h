
#ifndef UTILS_H_
#define UTILS_H_

#include "stdafx.h"
#include "dxhelper.h"

using Microsoft::WRL::ComPtr;

namespace helper
{

  // Gets an interface pointer from a Media Foundation collection.
template <class IFACE>
HRESULT utilGetCollectionObject(IMFCollection* collection, DWORD index, IFACE** ppObject)
{
  IUnknown* pUnk = nullptr;
  HRESULT hr = collection->GetElement(index, &pUnk);
  if (SUCCEEDED(hr))
  {
    hr = pUnk->QueryInterface(IID_PPV_ARGS(ppObject));
    pUnk->Release();
  }
  return hr;
}

inline HRESULT utilCopyAttribute(IMFAttributes* srcAttribute, IMFAttributes* dstAttribute, const GUID& key)
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
inline HRESULT utilCloneVideomediaType(IMFMediaType* srcMediaType, REFGUID guidSubType, IMFMediaType** ppNewMediaType)
{
  ComPtr<IMFMediaType> newMediaType = nullptr;

  HRESULT hr = MFCreateMediaType(newMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  hr = newMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = newMediaType->SetGUID(MF_MT_SUBTYPE, guidSubType);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_FRAME_SIZE);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_FRAME_RATE);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_INTERLACE_MODE);
  if (FAILED(hr))
  {
    return hr;
  }

  *ppNewMediaType = newMediaType.Get();
  (*ppNewMediaType)->AddRef();

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

inline HRESULT utilGetEncodingBitrate(IMFMediaType* mediaType, UINT32* encodingBitrate)
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

inline HRESULT utilConfigureVideoEncodeing(IMFCaptureSource* capSrc, IMFCaptureRecordSink* capRecord, REFGUID guidEncodingType)
{
  ComPtr<IMFMediaType> mediatype = nullptr;
  ComPtr<IMFMediaType> mediatype2 = nullptr;
  GUID guidSubType = GUID_NULL;

  // Configure the video format for the recording sink.
  HRESULT hr = capSrc->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, mediatype.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCloneVideomediaType(mediatype.Get(), guidEncodingType, mediatype2.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  hr = mediatype->GetGUID(MF_MT_SUBTYPE, &guidSubType);
  if (FAILED(hr))
  {
    return hr;
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
    hr = utilGetEncodingBitrate(mediatype2.Get(), &encodingBitrate);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = mediatype2->SetUINT32(MF_MT_AVG_BITRATE, encodingBitrate);
  }

  if (FAILED(hr))
  {
    return hr;
  }

  {
    // Connect the video stream to the recording sink.
    DWORD sinkStreamIndex = 0;
    hr = capRecord->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, mediatype2.Get(), nullptr, &sinkStreamIndex);
  }

  return hr;
}

inline HRESULT utilConfigureAudioEncoding(IMFCaptureSource* capSrc, IMFCaptureRecordSink* capRecord, REFGUID guidEncodingType)
{
  ComPtr<IMFCollection> availableTypes = nullptr;
  ComPtr<IMFMediaType> mediatype = nullptr;
  ComPtr<IMFAttributes> attributes = nullptr;

  // Configure the audio format for the recording sink.
  HRESULT hr = MFCreateAttributes(attributes.GetAddressOf(), 1);
  if (FAILED(hr))
  {
    return hr;
  }

  // Enumerate low latency media types.
  hr = attributes->SetUINT32(MF_LOW_LATENCY, true);
  if (FAILED(hr))
  {
    return hr;
  }

  // Get a list of encoded output formats that are supported by the encoder.
  hr = MFTranscodeGetAudioOutputAvailableTypes(
      guidEncodingType
      , MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER
      , attributes.Get()
      , availableTypes.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  // Pick the first format from the list.
  hr = utilGetCollectionObject(availableTypes.Get(), 0, mediatype.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  // Connect the audio stream to the recording sink.
  {
    DWORD sinkStreamIndex = 0;
    hr = capRecord->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, mediatype.Get(), nullptr, &sinkStreamIndex);
    if (hr == MF_E_INVALIDSTREAMNUMBER)
    {
      // If an audio device is not present, allow video only recording.
      hr = S_OK;
    }
  }

  return hr;
}

} // helper

#endif // UTILS_H_
