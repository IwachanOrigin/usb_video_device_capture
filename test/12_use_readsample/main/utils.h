
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

static inline HRESULT utilCopyAttribute(IMFAttributes* srcAttribute, IMFAttributes* dstAttribute, const GUID& key)
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

// Create a compatible format all items
static inline HRESULT utilCloneAllItems(IMFMediaType* srcMediaType, IMFMediaType** ppNewMediaType)
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

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_SUBTYPE);
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

// Create a compatible video format with a different subtype.
static inline HRESULT utilCloneVideomediaType(IMFMediaType* srcMediaType, REFGUID guidSubType, IMFMediaType** ppNewMediaType)
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

// Create a compatible audio format with a different subtype.
static inline HRESULT utilCloneAudiomediaType(IMFMediaType* srcMediaType, REFGUID guidSubType, IMFMediaType** ppNewMediaType)
{
  ComPtr<IMFMediaType> newMediaType = nullptr;

  HRESULT hr = MFCreateMediaType(newMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    return hr;
  }

  hr = newMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = newMediaType->SetGUID(MF_MT_SUBTYPE, guidSubType);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_AUDIO_NUM_CHANNELS);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_AUDIO_SAMPLES_PER_SECOND);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_AUDIO_BLOCK_ALIGNMENT);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = utilCopyAttribute(srcMediaType, newMediaType.Get(), MF_MT_AUDIO_BITS_PER_SAMPLE);
  if (FAILED(hr))
  {
    return hr;
  }

  *ppNewMediaType = newMediaType.Get();
  (*ppNewMediaType)->AddRef();

  return hr;
}


// Helper function to get the frame size from a video media type.
static inline HRESULT utilGetFrameSize(IMFMediaType* type, UINT32* width, UINT32* height)
{
  return MFGetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
}

// Helper function to get the frame rate from a video media type.
static inline HRESULT utilGetFrameRate(IMFMediaType* type, UINT32* numerator, UINT32* denominator)
{
  return MFGetAttributeRatio(type, MF_MT_FRAME_RATE, numerator, denominator);
}

static inline HRESULT utilGetEncodingBitrate(IMFMediaType* mediaType, UINT32* encodingBitrate)
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

} // helper

#endif // UTILS_H_
