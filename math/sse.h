// SSE helper routines
#pragma once

#include <other/core/math/copysign.h>
#include <other/core/math/isfinite.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <iostream>
#include <boost/type_traits/is_fundamental.hpp>
namespace other {

// Mark __m128 and __m128i as fundamental types
} namespace boost {
template<> struct is_fundamental<__m128> : public mpl::true_{};
template<> struct is_fundamental<__m128i> : public mpl::true_{};
} namespace other {

// fast_select(a,b,0) = a, fast_select(a,b,0xffffffff) = b, and anything else is undefined
static inline __m128i fast_select(__m128i a, __m128i b, __m128i mask) {
  // From http://markplusplus.wordpress.com/2007/03/14/fast-sse-select-operation
  return a^(mask&(a^b));
}

static inline __m128 fast_select(__m128 a, __m128 b, __m128i mask) {
  return (__m128)fast_select((__m128i)a,(__m128i)b,mask);
}

inline __m128 min(__m128 a, __m128 b) {
  return _mm_min_ps(a,b);
}

// This exist as a primitive in SSE4, but we do it ourselves to be SSE2 compatible
inline __m128i min(__m128i a, __m128i b) {
  return fast_select(a,b,_mm_cmpgt_epi32(a,b));
}

inline __m128 max(__m128 a, __m128 b) {
  return _mm_max_ps(a,b);
}

template<class T> struct pack_type;
template<> struct pack_type<float>{typedef __m128 type;};
template<> struct pack_type<int32_t>{typedef __m128i type;};
template<> struct pack_type<int64_t>{typedef __m128i type;};
template<> struct pack_type<uint32_t>{typedef __m128i type;};
template<> struct pack_type<uint64_t>{typedef __m128i type;};

// Same as _mm_set_ps, but without the bizarre reversed ordering
template<class T> static inline typename pack_type<T>::type pack(T x0, T x1);
template<class T> static inline typename pack_type<T>::type pack(T x0, T x1, T x2, T x3);

template<> inline __m128 pack<float>(float x0, float x1, float x2, float x3) {
  return _mm_set_ps(x3,x2,x1,x0);
}

template<> inline __m128i pack<int32_t>(int32_t x0, int32_t x1, int32_t x2, int32_t x3) {
  return _mm_set_epi32(x3,x2,x1,x0);
}

template<> inline __m128i pack<uint32_t>(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3) {
  return _mm_set_epi32(x3,x2,x1,x0);
}

template<> inline __m128i pack<int64_t>(int64_t x0, int64_t x1) {
  return _mm_set_epi64x(x1,x0);
}

template<> inline __m128i pack<uint64_t>(uint64_t x0, uint64_t x1) {
  return _mm_set_epi64x(x1,x0);
}

template<class D,class S> static inline D expand(S x);

template<> inline float expand(float x) {
  return x;
}

template<> inline __m128 expand(float x) {
  return _mm_set_ps1(x);
}

static inline __m128 sqrt(__m128 a) {
  return _mm_sqrt_ps(a);
}

static inline __m128 abs(__m128 a) {
  return (__m128)((__m128i)a&_mm_set1_epi32(~(1<<31)));
}

static inline __m128i isnotfinite(__m128 a) {
  const __m128i exponent = _mm_set1_epi32(0xff<<23);
  return _mm_cmpeq_epi32((__m128i)a&exponent,exponent);
}

static inline __m128i isfinite(__m128 a) {
  return ~isnotfinite(a);
}

static inline __m128 copysign(__m128 mag, __m128 sign) {
  return (__m128)(((__m128i)mag&_mm_set1_epi32(~(1<<31)))|((__m128i)sign&_mm_set1_epi32(1<<31)));
}

static inline std::ostream& operator<<(std::ostream& os, __m128 a) {
  float x[4];
  _mm_store_ps(x,a);
  return os<<'['<<x[0]<<','<<x[1]<<','<<x[2]<<','<<x[3]<<']';
}

static inline std::ostream& operator<<(std::ostream& os, __m128i a) {
  int x[4];
  *(__m128i*)x = a;
  return os<<'['<<x[0]<<','<<x[1]<<','<<x[2]<<','<<x[3]<<']';
}

static inline void transpose(__m128i& i0, __m128i& i1, __m128i& i2, __m128i& i3) {
  __m128 f0 = (__m128)i0,
         f1 = (__m128)i1,
         f2 = (__m128)i2,
         f3 = (__m128)i3;
  _MM_TRANSPOSE4_PS(f0,f1,f2,f3);
  i0 = (__m128i)f0;
  i1 = (__m128i)f1;
  i2 = (__m128i)f2;
  i3 = (__m128i)f3;
}

}