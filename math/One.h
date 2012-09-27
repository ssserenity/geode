//#####################################################################
// Class One
//#####################################################################
#pragma once

#include <other/core/math/Zero.h>
#include <iostream>
namespace other {

struct One {
  bool operator!() const {
    return false;
  }

  One operator*(const One) const {
    return One();
  }

  bool operator==(const One) const {
    return true;
  }

  One inverse() const {
    return One();
  }

  static One one() {
    return One();
  }
};

template<class T> static inline const T& operator*(const T& x, const One) {
  return x;
}

template<class T> static inline const T& operator*(const One, const T& x) {
  return x;
}

template<class T> static inline const T& operator/(const T& x, const One) {
  return x;
}

template<class T> static inline T& operator*=(T& x, const One) {
  return x;
}

template<class T> static inline T& operator/=(T& x, const One) {
  return x;
}

static inline std::ostream& operator<<(std::ostream& output, const One) {
  return output<<'1';
}

}