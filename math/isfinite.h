#pragma once

#include <cmath>
namespace other {
  
#ifdef Win32
  
template<class T> static inline bool isfinite(T x) {
  return _isfinite(x);
}
  
#else
  
#undef isfinite
using std::isfinite;
  
#endif
}