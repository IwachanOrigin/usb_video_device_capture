
#include <iostream>
#include <string>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include "MFUtility.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

typedef enum _VideoFormatType {
    VFT_AI44,
    VFT_ARGB32,
    VFT_AYUV,
    VFT_DV25,
    VFT_DV50,
    VFT_DVH1,
    VFT_DVSD,
    VFT_DVSL,
    VFT_H264,
    VFT_I420,
    VFT_IYUV,
    VFT_M4S2,
    VFT_MJPG,
    VFT_MP43,
    VFT_MP4S,
    VFT_MP4V,
    VFT_MPG1,
    VFT_MSS1,
    VFT_MSS2,
    VFT_NV11,
    VFT_NV12,
    VFT_P010,
    VFT_P016,
    VFT_P210,
    VFT_P216,
    VFT_RGB24,
    VFT_RGB32,
    VFT_RGB555,
    VFT_RGB565,
    VFT_RGB8,
    VFT_UYVY,
    VFT_v210,
    VFT_v410,
    VFT_WMV1,
    VFT_WMV2,
    VFT_WMV3,
    VFT_WVC1,
    VFT_Y210,
    VFT_Y216,
    VFT_Y410,
    VFT_Y416,
    VFT_Y41P,
    VFT_Y41T,
    VFT_YUY2,
    VFT_YV12,
    VFT_YVYU,
    VFT_NOT_SUPPORTED
} VideoFormatType;

static UINT32 image_width  = 0;
static UINT32 image_height = 0;
static  INT32 image_stride = 0;
static UINT32 image_length = 0;
static BYTE *image_data = NULL;
static VideoFormatType image_format = VFT_NOT_SUPPORTED;

HRESULT CreateVideoSource(IMFMediaSource** source)
{
  IMFMediaSource* src = nullptr;
  IMFAttributes* attr = nullptr;
  IMFActivate** devices = nullptr;
  HRESULT hr = S_OK;
  UINT32 i = 0, count = 0;

  hr = MFCreateAttributes(&attr, 1);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = MFEnumDeviceSources(attr, &devices, &count);
  if (FAILED(hr))
  {
    goto done;
  }

  if (count == 0)
  {
    hr = E_FAIL;
    goto done;
  }

  hr = devices[0]->ActivateObject(IID_PPV_ARGS(&src));
  if (FAILED(hr))
  {
    goto done;
  }

  *source = src;

done:
  SAFE_RELEASE(&attr);
  for (i = 0; i < count; i++)
  {
    SAFE_RELEASE(&devices[i]);
  }
  CoTaskMemFree(devices);

  return hr;
}

HRESULT GetCurrentType(IMFMediaSource* source, IMFMediaType** type)
{
  HRESULT hr = S_OK;
  BOOL selected = FALSE;
  IMFPresentationDescriptor* presDesc = nullptr;
  IMFStreamDescriptor* strmDesc = nullptr;
  IMFMediaTypeHandler* handler = nullptr;

  *type = nullptr;

  hr = source->CreatePresentationDescriptor(&presDesc);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = presDesc->GetStreamDescriptorByIndex(0, &selected, &strmDesc);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = strmDesc->GetMediaTypeHandler(&handler);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = handler->GetCurrentMediaType(type);
  if (FAILED(hr))
  {
    goto done;
  }

done:
  SAFE_RELEASE(&presDesc);
  SAFE_RELEASE(&strmDesc);
  SAFE_RELEASE(&handler);

  return hr;
}

HRESULT IsVideoType(IMFMediaType* type, BOOL* b)
{
  HRESULT hr = S_OK;
  PROPVARIANT var;

  *b = FALSE;

  PropVariantInit(&var);

  hr = type->GetItem(MF_MT_MAJOR_TYPE, &var);
  if (FAILED(hr))
  {
    return hr;
  }

  if (var.vt == VT_CLSID)
  {
    const GUID &guid = *var.puuid;
    if (guid == MFMediaType_Video)
    {
      *b = TRUE;
    }
  }

  return hr;
}

HRESULT GetFrameSize(IMFMediaType *type, UINT32 *width, UINT32 *height)
{
  HRESULT hr;
  PROPVARIANT var;

  *width  = 0;
  *height = 0;

  PropVariantInit(&var);

  hr = type->GetItem(MF_MT_FRAME_SIZE, &var);
  if(FAILED(hr)) return hr;

  Unpack2UINT32AsUINT64(var.uhVal.QuadPart, width, height);

  return hr;
}

HRESULT GetStride(IMFMediaType *type, INT32 *stride)
{
  HRESULT hr;
  PROPVARIANT var;

  *stride = 0;

  PropVariantInit(&var);

  hr = type->GetItem(MF_MT_DEFAULT_STRIDE, &var);
  if(FAILED(hr)) return hr;

  *stride = (INT32)var.ulVal;

  return hr;
}

HRESULT GetSampleSize(IMFMediaType *type, UINT32 *length)
{
  HRESULT hr;
  PROPVARIANT var;

  *length = 0;

  PropVariantInit(&var);

  hr = type->GetItem(MF_MT_SAMPLE_SIZE, &var);
  if(FAILED(hr)) return hr;

  *length = var.ulVal;

  return hr;
}

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return #val
#endif

static LPCSTR GetFormatName(const VideoFormatType &format) {
    IF_EQUAL_RETURN(format, VFT_AI44);//      FCC('AI44')
    IF_EQUAL_RETURN(format, VFT_ARGB32); //   D3DFMT_A8R8G8B8
    IF_EQUAL_RETURN(format, VFT_AYUV); //     FCC('AYUV')
    IF_EQUAL_RETURN(format, VFT_DV25); //     FCC('dv25')
    IF_EQUAL_RETURN(format, VFT_DV50); //     FCC('dv50')
    IF_EQUAL_RETURN(format, VFT_DVH1); //     FCC('dvh1')
    IF_EQUAL_RETURN(format, VFT_DVSD); //     FCC('dvsd')
    IF_EQUAL_RETURN(format, VFT_DVSL); //     FCC('dvsl')
    IF_EQUAL_RETURN(format, VFT_H264); //     FCC('H264')
    IF_EQUAL_RETURN(format, VFT_I420); //     FCC('I420')
    IF_EQUAL_RETURN(format, VFT_IYUV); //     FCC('IYUV')
    IF_EQUAL_RETURN(format, VFT_M4S2); //     FCC('M4S2')
    IF_EQUAL_RETURN(format, VFT_MJPG);
    IF_EQUAL_RETURN(format, VFT_MP43); //     FCC('MP43')
    IF_EQUAL_RETURN(format, VFT_MP4S); //     FCC('MP4S')
    IF_EQUAL_RETURN(format, VFT_MP4V); //     FCC('MP4V')
    IF_EQUAL_RETURN(format, VFT_MPG1); //     FCC('MPG1')
    IF_EQUAL_RETURN(format, VFT_MSS1); //     FCC('MSS1')
    IF_EQUAL_RETURN(format, VFT_MSS2); //     FCC('MSS2')
    IF_EQUAL_RETURN(format, VFT_NV11); //     FCC('NV11')
    IF_EQUAL_RETURN(format, VFT_NV12); //     FCC('NV12')
    IF_EQUAL_RETURN(format, VFT_P010); //     FCC('P010')
    IF_EQUAL_RETURN(format, VFT_P016); //     FCC('P016')
    IF_EQUAL_RETURN(format, VFT_P210); //     FCC('P210')
    IF_EQUAL_RETURN(format, VFT_P216); //     FCC('P216')
    IF_EQUAL_RETURN(format, VFT_RGB24); //    D3DFMT_R8G8B8
    IF_EQUAL_RETURN(format, VFT_RGB32); //    D3DFMT_X8R8G8B8
    IF_EQUAL_RETURN(format, VFT_RGB555); //   D3DFMT_X1R5G5B5
    IF_EQUAL_RETURN(format, VFT_RGB565); //   D3DFMT_R5G6B5
    IF_EQUAL_RETURN(format, VFT_RGB8);
    IF_EQUAL_RETURN(format, VFT_UYVY); //     FCC('UYVY')
    IF_EQUAL_RETURN(format, VFT_v210); //     FCC('v210')
    IF_EQUAL_RETURN(format, VFT_v410); //     FCC('v410')
    IF_EQUAL_RETURN(format, VFT_WMV1); //     FCC('WMV1')
    IF_EQUAL_RETURN(format, VFT_WMV2); //     FCC('WMV2')
    IF_EQUAL_RETURN(format, VFT_WMV3); //     FCC('WMV3')
    IF_EQUAL_RETURN(format, VFT_WVC1); //     FCC('WVC1')
    IF_EQUAL_RETURN(format, VFT_Y210); //     FCC('Y210')
    IF_EQUAL_RETURN(format, VFT_Y216); //     FCC('Y216')
    IF_EQUAL_RETURN(format, VFT_Y410); //     FCC('Y410')
    IF_EQUAL_RETURN(format, VFT_Y416); //     FCC('Y416')
    IF_EQUAL_RETURN(format, VFT_Y41P);
    IF_EQUAL_RETURN(format, VFT_Y41T);
    IF_EQUAL_RETURN(format, VFT_YUY2); //     FCC('YUY2')
    IF_EQUAL_RETURN(format, VFT_YV12); //     FCC('YV12')
    IF_EQUAL_RETURN(format, VFT_YVYU);
    IF_EQUAL_RETURN(format, VFT_NOT_SUPPORTED);
    return NULL;
}


#ifndef FORMAT_MATCH
#define FORMAT_MATCH(param, val, type) if(val == param) return type
#endif

static VideoFormatType GetFormatType(const GUID &guid) {
    FORMAT_MATCH(guid, MFVideoFormat_AI44, VFT_AI44);
    FORMAT_MATCH(guid, MFVideoFormat_ARGB32, VFT_ARGB32);
    FORMAT_MATCH(guid, MFVideoFormat_AYUV, VFT_AYUV);
    FORMAT_MATCH(guid, MFVideoFormat_DV25, VFT_DV25);
    FORMAT_MATCH(guid, MFVideoFormat_DV50, VFT_DV50);
    FORMAT_MATCH(guid, MFVideoFormat_DVH1, VFT_DVH1);
    FORMAT_MATCH(guid, MFVideoFormat_DVSD, VFT_DVSD);
    FORMAT_MATCH(guid, MFVideoFormat_DVSL, VFT_DVSL);
    FORMAT_MATCH(guid, MFVideoFormat_H264, VFT_H264);
    FORMAT_MATCH(guid, MFVideoFormat_I420, VFT_I420);
    FORMAT_MATCH(guid, MFVideoFormat_IYUV, VFT_IYUV);
    FORMAT_MATCH(guid, MFVideoFormat_M4S2, VFT_M4S2);
    FORMAT_MATCH(guid, MFVideoFormat_MJPG, VFT_MJPG);
    FORMAT_MATCH(guid, MFVideoFormat_MP43, VFT_MP43);
    FORMAT_MATCH(guid, MFVideoFormat_MP4S, VFT_MP4S);
    FORMAT_MATCH(guid, MFVideoFormat_MP4V, VFT_MP4V);
    FORMAT_MATCH(guid, MFVideoFormat_MPG1, VFT_MPG1);
    FORMAT_MATCH(guid, MFVideoFormat_MSS1, VFT_MSS1);
    FORMAT_MATCH(guid, MFVideoFormat_MSS2, VFT_MSS2);
    FORMAT_MATCH(guid, MFVideoFormat_NV11, VFT_NV11);
    FORMAT_MATCH(guid, MFVideoFormat_NV12, VFT_NV12);
    FORMAT_MATCH(guid, MFVideoFormat_P010, VFT_P010);
    FORMAT_MATCH(guid, MFVideoFormat_P016, VFT_P016);
    FORMAT_MATCH(guid, MFVideoFormat_P210, VFT_P210);
    FORMAT_MATCH(guid, MFVideoFormat_P216, VFT_P216);
    FORMAT_MATCH(guid, MFVideoFormat_RGB24, VFT_RGB24);
    FORMAT_MATCH(guid, MFVideoFormat_RGB32, VFT_RGB32);
    FORMAT_MATCH(guid, MFVideoFormat_RGB555, VFT_RGB555);
    FORMAT_MATCH(guid, MFVideoFormat_RGB565, VFT_RGB565);
    FORMAT_MATCH(guid, MFVideoFormat_RGB8, VFT_RGB8);
    FORMAT_MATCH(guid, MFVideoFormat_UYVY, VFT_UYVY);
    FORMAT_MATCH(guid, MFVideoFormat_v210, VFT_v210);
    FORMAT_MATCH(guid, MFVideoFormat_v410, VFT_v410);
    FORMAT_MATCH(guid, MFVideoFormat_WMV1, VFT_WMV1);
    FORMAT_MATCH(guid, MFVideoFormat_WMV2, VFT_WMV2);
    FORMAT_MATCH(guid, MFVideoFormat_WMV3, VFT_WMV3);
    FORMAT_MATCH(guid, MFVideoFormat_WVC1, VFT_WVC1);
    FORMAT_MATCH(guid, MFVideoFormat_Y210, VFT_Y210);
    FORMAT_MATCH(guid, MFVideoFormat_Y216, VFT_Y216);
    FORMAT_MATCH(guid, MFVideoFormat_Y410, VFT_Y410);
    FORMAT_MATCH(guid, MFVideoFormat_Y416, VFT_Y416);
    FORMAT_MATCH(guid, MFVideoFormat_Y41P, VFT_Y41P);
    FORMAT_MATCH(guid, MFVideoFormat_Y41T, VFT_Y41T);
    FORMAT_MATCH(guid, MFVideoFormat_YUY2, VFT_YUY2);
    FORMAT_MATCH(guid, MFVideoFormat_YV12, VFT_YV12);
    FORMAT_MATCH(guid, MFVideoFormat_YVYU, VFT_YVYU);
    return VFT_NOT_SUPPORTED;
}

HRESULT GetVideoFormat(IMFMediaType *type, VideoFormatType *format)
{
  HRESULT hr;
  PROPVARIANT var;

  PropVariantInit(&var);

  hr = type->GetItem(MF_MT_SUBTYPE, &var);
  if(FAILED(hr)) return hr;

  if(var.vt == VT_CLSID) {
    *format = GetFormatType(*var.puuid);
  } else {
    hr = E_FAIL;
  }

  return hr;
}

HRESULT GetMediaFormat(IMFMediaType *type)
{
  HRESULT hr;
  BOOL isVideo;

  hr = IsVideoType(type, &isVideo);
  if(!isVideo) return E_FAIL;

  printf("Media Type\n");

  hr = GetVideoFormat(type, &image_format);
  if(SUCCEEDED(hr)) {
    printf("  Video format = %s\n", GetFormatName(image_format));
  }

  hr = GetFrameSize(type, &image_width, &image_height);
  if(SUCCEEDED(hr)) {
    printf("  Frame size   = %d x %d\n", image_width, image_height);
  }

  hr = GetStride(type, &image_stride);
  if(SUCCEEDED(hr)) {
    printf("  Stride       = %d\n", image_stride);
  }

  hr = GetSampleSize(type, &image_length);
  if(SUCCEEDED(hr)) {
    printf("  Sample Size  = %d\n", image_length);
  }

  return hr;
}

HRESULT CreateSourceReader(IMFMediaSource *source, IMFSourceReader **reader)
{
  IMFSourceReader *p;
  HRESULT hr;

  *reader = NULL;

  hr = MFCreateSourceReaderFromMediaSource(source, NULL, &p);
  if(FAILED(hr)) {
    printf("Failed create IMFSourceReader object\n");
    return hr;
  }

  *reader = p;

  return S_OK;
}

HRESULT GetBufferData(IMFSample *sample)
{
  HRESULT hr;
  IMFMediaBuffer *buff;
  DWORD maxLen, curLen;
  BYTE *memory;

  hr = sample->GetBufferByIndex(0, &buff);
  if(FAILED(hr)) {
    return hr;
  }

  hr = buff->Lock(&memory, &maxLen, &curLen);
  if(FAILED(hr)) goto done;

  printf("  Max Length   = %d\n", maxLen);
  printf("  Curr Length  = %d\n", curLen);
  printf("  Address      = %#x\n", memory);

  hr = buff->Unlock();

done:
  SAFE_RELEASE(&buff);
  return hr;
}

static void MainLoop(IMFSourceReader *reader)
{
  IMFSample *sample = nullptr;
  DWORD streamIndex = 0, flags = 0;
  LONGLONG timeStamp = 0;
  HRESULT hr;

  for(int i=0; i<10; i++)
  {
    hr = reader->ReadSample(
      (DWORD)MF_SOURCE_READER_ANY_STREAM,
      0,
      &streamIndex,
      &flags,
      &timeStamp,
      &sample);

    if(SUCCEEDED(hr))
    {
      printf("Frame %d:\n", i);
      printf("  Stream Index = %u\n", streamIndex);
      printf("  Flags        = %u\n", flags);
      printf("  Time Stamp   = %u\n", timeStamp);

      if(sample)
      {
        GetBufferData(sample);
      }
      SAFE_RELEASE(&sample);
    }
  }
}


int main(int argc, char* argv[])
{
  // Initialize
  HRESULT hr;
  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if(FAILED(hr))
  {
    printf("Failed initializing COM components\n");
    exit(1);
  }
  
  hr = MFStartup(MF_VERSION);
  if (FAILED(hr))
  {
    return hr;
  }

  IMFMediaSource *source = nullptr;
  IMFMediaType   *type = nullptr;

  if(SUCCEEDED(CreateVideoSource(&source)) &&
     SUCCEEDED(GetCurrentType(source, &type)) &&
     SUCCEEDED(GetMediaFormat(type)))
  {
    IMFSourceReader *reader;

    hr = CreateSourceReader(source, &reader);
    if(SUCCEEDED(hr))
    {
      MainLoop(reader);

      SAFE_RELEASE(&reader);
      SAFE_RELEASE(&type);
      SAFE_RELEASE(&source);
    }
    else
    {
      /*
       *   Some Microsoft Media Foundation objects must be shut
       * down before being released. If so, the caller is
       * responsible for shutting down the object that is returned
       * in ppv. To shut down the object, do one of the following:
       *
       * + Call IMFActivate::ShutdownObject on the activation object
       *
       * + Call the object-specific shutdown method. This method will
       *   depend on the type of object.
       */
      source->Shutdown();

      SAFE_RELEASE(&type);
      SAFE_RELEASE(&source);
    }
  }

  // Release
  hr = MFShutdown();
  if (FAILED(hr))
  {
    return hr;
  }

  CoUninitialize();

  return 0;
}
