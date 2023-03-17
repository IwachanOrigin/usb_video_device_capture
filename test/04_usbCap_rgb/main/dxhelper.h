
#ifndef DX_HELPER_H_
#define DX_HELPER_H_

#include "stdafx.h"
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace dx_engine
{

inline std::string HrToString(HRESULT hr)
{
  char s_str[64] = {};
  sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
  return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
  HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
  HRESULT Error() const { return m_hr; }
private:
  const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
  if (FAILED(hr))
  {
    throw HrException(hr);
  }
}

template <class T> void SAFE_RELEASE(T** ppT)
{
  if (*ppT)
  {
    (*ppT)->Release();
    *ppT = NULL;
  }
}

template <class T> inline void SAFE_RELEASE(T*& pT)
{
  if (pT != NULL)
  {
    pT->Release();
    pT = NULL;
  }
}

template <class T> inline void SAFE_RELEASE(T t)
{
  if (t)
  {
    t->Release();
  }
}

} // dx_engine

#endif // DX_HELPER_H_
