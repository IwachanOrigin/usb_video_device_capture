
#ifndef CAPTURE_MANAGER_H_
#define CAPTURE_MANAGER_H_

#include "stdafx.h"
#include "dxhelper.h"
#include <intsafe.h>
#include <winnt.h>

class CaptureManager
{
  // The event callback object.
  class CaptureEngineCB : public IMFCaptureEngineOnEventCallback
  {
    long m_ref;
    HWND m_hwnd;

  public:
    CaptureEngineCB(HWND hwnd) : m_ref(1), m_hwnd(hwnd), m_isSleeping(false), m_manager(nullptr) {}

    // IUnknown
    STDMETHODIMP  QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_ (ULONG) AddRef();
    STDMETHODIMP_ (ULONG) Release();

    // IMFCaptureEngineOnEventCallback
    STDMETHODIMP OnEvent(_In_ IMFMediaEvent* event);

    void setSleeping(const bool sleeping) { m_isSleeping = sleeping; }
    bool isSleeping() { return m_isSleeping; }
  private:
    bool m_isSleeping;
    CaptureManager* m_manager;
  };

  HWND m_hwndEvent;

  IMFCaptureEngine* m_captureEngine;
  IMFCapturePreviewSink *m_capPrevSink;
  CaptureEngineCB* m_capCallback;
  bool m_isPreviewing;
  bool m_isRecording;
  bool m_isPhotoPending;

  UINT m_errorID;
  HANDLE m_event;
  HANDLE m_pwrRequest;
  bool m_isPowerRequestSet;

  CaptureManager(HWND hwnd)
  {
    m_hwndEvent = hwnd;
    m_captureEngine = nullptr;
    m_capPrevSink = nullptr;
    m_capCallback = nullptr;
    m_isRecording = false;
    m_isPreviewing = false;
    m_isPhotoPending = false;
    m_errorID = 0;
    m_event = nullptr;
    m_pwrRequest = INVALID_HANDLE_VALUE;
    m_isPowerRequestSet = false;

    REASON_CONTEXT powerContext = {};
    powerContext.Version = POWER_REQUEST_CONTEXT_VERSION;
    powerContext.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
    powerContext.Reason.SimpleReasonString = L"CaptureEngine is recording!";

    m_pwrRequest = PowerCreateRequest(&powerContext);
  }

  void setErrorID(HRESULT hr, UINT id)
  {
    m_errorID = SUCCEEDED(hr) ? 0 : id;
  }

  // Capture engine event handlers
  void onCapEngineInit(HRESULT& hr);
  void onPreviewStarted(HRESULT& hr);
  void onPreviewStopped(HRESULT& hr);
  void onRecordStarted(HRESULT& hr);
  void onRecordStopped(HRESULT& hr);
  void waitForResult()
  {
    WaitForSingleObject(m_event, INFINITE);
  }

public:
  ~CaptureManager()
  {
    this->destroyCapEngine();
  }

  static HRESULT createInst(HWND hwndEvent, CaptureManager** ppEngine)
  {
    HRESULT hr = S_OK;
    ppEngine = nullptr;

    CaptureManager* pEngine = new (std::nothrow)CaptureManager(hwndEvent);
    if (pEngine != nullptr)
    {
      *ppEngine = pEngine;
      pEngine = nullptr;
    }
    else
    {
      hr = E_OUTOFMEMORY;
    }
    return hr;
  }

  HRESULT initCaptureManager(IUnknown* pUnk);
  void destroyCapEngine()
  {
    if (nullptr != m_event)
    {
      CloseHandle(m_event);
      m_event = nullptr;
    }

    SAFE_RELEASE(m_capPrevSink);
    SAFE_RELEASE(m_captureEngine);
    SAFE_RELEASE(m_capCallback);

    m_isPreviewing = false;
    m_isRecording = false;
    m_isPhotoPending = false;
    m_errorID = 0;
  }

  bool isPreviewing() const { return m_isPreviewing; }
  bool isRecording() const { return m_isRecording; }
  bool isPhotoPending() const { return m_isPhotoPending; }
  UINT errorID() const { return m_errorID; }

  HRESULT onCapEvent(WPARAM wParam, LPARAM lParam);
  HRESULT setVideoDevice(IUnknown* pUnk);
  HRESULT startPreview();
  HRESULT stopPreview();
  HRESULT startRecord(PCWSTR destinationFile);
  HRESULT stopRecord();
  HRESULT takePhoto(PCWSTR filename);

  void sleepState(bool sleeping)
  {
    if (nullptr != m_capCallback)
    {
      m_capCallback->setSleeping(sleeping);
    }
  }

  HRESULT updateVideo()
  {
    if (m_capPrevSink)
    {
      return m_capPrevSink->UpdateVideo(nullptr, nullptr, nullptr);
    }
    else
    {
      return S_OK;
    }
  }
};

#endif // CAPTURE_MANAGER_H_
