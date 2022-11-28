
#ifndef CAPTURE_MANAGER_H_
#define CAPTURE_MANAGER_H_

#include "stdafx.h"
#include "dxhelper.h"

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
};

#endif // CAPTURE_MANAGER_H_
