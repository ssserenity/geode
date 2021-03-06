// Multiprecision integer arithmetic for exact geometric predicates
#pragma once

#include <geode/config.h>
#ifndef GEODE_GMP
#error geode/exact requires gmp support: recompile with use_gmp=1 or check config.log for gmp errors
#endif

#include <geode/exact/config.h>
#include <geode/array/RawArray.h>
#include <geode/math/One.h>
#include <geode/math/integer_log.h>
#include <geode/utility/endian.h>
#include <geode/utility/move.h>
#include <gmp.h>
namespace geode {

#if GMP_LIMB_BITS==64 && defined(__GNUC__)
#define GEODE_FAST_EXACT 1
#define ASSERT_LARGE(a) \
  static_assert((a)>=32,"In fast mode, GMP should be used only in test routines")
#else
#define ASSERT_LARGE(a)
#endif

// If we have gmp 4, set up some compatibility routines
#if __GNU_MP__ < 5
static inline void mpn_neg(mp_limb_t* rp, const mp_limb_t* sp, mp_size_t n) {
  mpn_sub_1(rp,sp,n,1);
  for (int i=0;i<int(n);i++)
    rp[i] = ~rp[i];
}
static inline void mpn_sqr(mp_limb_t* rp, const mp_limb_t* sp, mp_size_t n) {
  mpn_mul_n(rp,sp,sp,n);
}
#endif

// A fixed width 2's complement integer compatible with GMP's low level interface.
// See http://gmplib.org/manual/Low_002dlevel-Functions.html#Low_002dlevel-Functions.
// Exact<d> holds a signed integer with exactly d*sizeof(Quantized) bytes, suitable
// for representing the values of polynomial predicates of degree d.
template<int d> struct Exact {
  static_assert(d>=1,"");
  static const int degree = d;
  static const int ratio = sizeof(Quantized)/sizeof(mp_limb_t);
  static_assert(sizeof(Quantized)==ratio*sizeof(mp_limb_t),"Limb counts must be integral");
  static const int limbs = d*ratio;

  // 2's complement, little endian array of GMP limbs
  mp_limb_t n[limbs];

  Exact() {
    memset(n,0,sizeof(n));
  }

  explicit Exact(Uninit) {}

  explicit Exact(const ExactInt x) {
    static_assert(d==1 && sizeof(x)==sizeof(n) && limbs<=2,"");
    memcpy(n,&x,sizeof(x));
    if (GEODE_ENDIAN==GEODE_BIG_ENDIAN && limbs==2) // Convert from big endian limb order to little endian
      swap(n[0],n[1]);
  }

  template<int smaller_d> explicit Exact(const Exact<smaller_d>& rhs) {
    static_assert(d > smaller_d, "Can only assign if increasing precision"); // should fall back to default operator for a == b
    memcpy(n,&rhs.n,sizeof(rhs.n));
    memset(n + smaller_d, is_negative(rhs) ? 0xFF : 0, (d - smaller_d)*sizeof(n[0]));
  }
};

template<int d> static inline bool is_negative(const Exact<d>& x) {
  return (mp_limb_signed_t(x.n[x.limbs-1])<0);
}

template<int d> static inline bool is_nonzero(const Exact<d>& x) {
  for (int i=0;i<x.limbs;i++)
    if (x.n[i])
      return true;
  return false;
}

template<int d> static inline bool operator==(const Exact<d>& lhs, const Exact<d>& rhs) {
  if(Exact<d>::limbs == 1)
    return lhs.n[0] == rhs.n[0];
  else
    return mpn_cmp(lhs.n, rhs.n, Exact<d>::limbs) == 0;
}

// For template compatibility with Interval
template<int d> static inline bool overlap(const Exact<d>& x, const Exact<d>& y) {
  return x==y;
}

template<int d> static inline int sign(const Exact<d>& x) {
  if (is_negative(x))
    return -1;
  return is_nonzero(x);
}

template<int d> static inline bool operator<(const Exact<d>& lhs, const Exact<d>& rhs) {
  return sign(lhs - rhs) < 0;
}
template<int d> static inline bool operator<=(const Exact<d>& lhs, const Exact<d>& rhs) {
  return sign(lhs - rhs) <= 0;
}
template<int d> static inline bool operator>=(const Exact<d>& lhs, const Exact<d>& rhs) {
  return sign(lhs - rhs) >= 0;
}
template<int d> static inline bool operator>(const Exact<d>& lhs, const Exact<d>& rhs) {
  return sign(lhs - rhs) > 0;
}

#if GEODE_FAST_EXACT

// Pull in autogenerated arithmetic routines
#include <geode/exact/exact-generated.h>

#endif // GEODE_FAST_EXACT

template<int a> GEODE_PURE static inline Exact<a> operator+(const Exact<a> x, const Exact<a> y) {
  ASSERT_LARGE(a);
  Exact<a> r(uninit);
  if (r.limbs==1)
    r.n[0] = x.n[0] + y.n[0];
  else
    mpn_add_n(r.n,x.n,y.n,r.limbs);
  return r;
}

template<int a> static inline typename enable_if_c<(a<=32)>::type operator+=(Exact<a>& x, const Exact<a>& y) {
  x = x+y;
}

template<int a> static inline typename enable_if_c<(a>32)>::type operator+=(Exact<a>& x, const Exact<a>& y) {
  ASSERT_LARGE(a);
  mpn_add_n(x.n,x.n,y.n,x.limbs);
}

template<int a> GEODE_PURE static inline Exact<a> operator-(const Exact<a> x, const Exact<a> y) {
  ASSERT_LARGE(a);
  Exact<a> r(uninit);
  if (r.limbs==1)
    r.n[0] = x.n[0] - y.n[0];
  else
    mpn_sub_n(r.n,x.n,y.n,r.limbs);
  return r;
}

template<int a,int b> GEODE_PURE static inline typename enable_if_c<(a>=b),Exact<a+b>>::type
operator*(const Exact<a> x, const Exact<b> y) {
  ASSERT_LARGE(a);
  ASSERT_LARGE(b);
  // Perform multiplication as if inputs were unsigned
  Exact<a+b> r(uninit);
  if (a>=b)
    mpn_mul(r.n,x.n,x.limbs,y.n,y.limbs);
  else
    mpn_mul(r.n,y.n,y.limbs,x.n,x.limbs);
  // Correct for negative numbers
  if (is_negative(x)) {
    if (y.limbs==1)
      r.n[x.limbs] -= y.n[0];
    else
      mpn_sub_n(r.n+x.limbs,r.n+x.limbs,y.n,y.limbs);
  }
  if (is_negative(y)) {
    if (x.limbs==1)
      r.n[y.limbs] -= x.n[0];
    else
      mpn_sub_n(r.n+y.limbs,r.n+y.limbs,x.n,x.limbs);
  }
  return r;
}

// Multiplication is symmetric
template<int a,int b> GEODE_PURE static inline typename enable_if_c<(a<b),Exact<a+b>>::type
operator*(const Exact<a> x, const Exact<b> y) {
  return y*x;
}

template<int a> GEODE_PURE static inline Exact<2*a> sqr(const Exact<a> x) {
  ASSERT_LARGE(a);
  Exact<2*a> r(uninit);
  mp_limb_t nx[x.limbs];
  const bool negative = is_negative(x);
  if (negative)
    mpn_neg(nx,x.n,x.limbs);
  mpn_sqr(r.n,negative?nx:x.n,x.limbs);
  return r;
}

template<int a> GEODE_PURE static inline Exact<a> operator<<(const Exact<a> x, const int s) {
  ASSERT_LARGE(a);
  assert(0<s && s<=3);
  Exact<a> r(uninit);
  if (x.limbs==1)
    r.n[0] = x.n[0] << s;
  else
    mpn_lshift(r.n,x.n,x.limbs,s);
  return r;
}

template<int a> GEODE_PURE static inline Exact<a> operator-(const Exact<a> x) {
  Exact<a> r(uninit);
  if (r.limbs==1)
    r.n[0] = mp_limb_t(-mp_limb_signed_t(x.n[0]));
  else
    mpn_neg(r.n,x.n,x.limbs);
  return r;
}

template<int a> GEODE_PURE static inline Exact<3*a> cube(const Exact<a> x) {
  return x*sqr(x);
}

template<int a> static inline void operator++(Exact<a>& x) {
  ASSERT_LARGE(Exact<a>::limbs==1 ? 32 : a);
  if (x.limbs==1)
    x.n[0]++;
  else
    mpn_add_1(x.n,x.n,x.limbs,1);
}

#undef ASSERT_LARGE

// Truncating operations

template<int a> GEODE_PURE static inline Exact<(a>>1) + (a&1)> ceil_sqrt(const Exact<a> x) {
  assert(!is_negative(x));
  Exact<(a>>1)+(a&1)> r;
  const auto trim_x = trim(asarray(x.n));
  const auto inexact = mpn_sqrtrem(r.n,0,trim_x.data(),trim_x.size());
  if (inexact)
    ++r;
  return r;
}

template<int a> static inline Exact<a>& operator>>=(Exact<a>& x, const int s) {
  assert(!is_negative(x));
  assert(0<s && s<=3);
  mpn_rshift(x.n,x.n,x.limbs,s);
  return x;
}

template<int a> GEODE_PURE static inline Exact<a> operator>>(const Exact<a> x, const int s) {
  Exact<1> r = x;
  r >>= s;
  return r;
}

// Signs of integers in limb array form

static inline bool mpz_negative(RawArray<const mp_limb_t> x) {
  return mp_limb_signed_t(x.back())<0;
}

static inline int mpz_sign(RawArray<const mp_limb_t> x) {
  if (mp_limb_signed_t(x.back())<0)
    return -1;
  for (int i=0;i<x.size();i++)
    if (x[i])
      return 1;
  return 0;
}

static inline bool mpz_nonzero(RawArray<const mp_limb_t> x) {
  return !x.contains_only(0);
}

// Copy from Exact<d> to an array with sign extension

static inline void mpz_set(RawArray<mp_limb_t> x, RawArray<const mp_limb_t> y) {
  x.slice(0,y.size()) = y;
  x.slice(y.size(),x.size()).fill(mp_limb_signed_t(y.back())>=0 ? 0 : mp_limb_t(mp_limb_signed_t(-1)));
}

static inline void mpz_set_nonnegative(RawArray<mp_limb_t> x, RawArray<const mp_limb_t> y) {
  assert(mp_limb_signed_t(y.back())>=0);
  x.slice(0,y.size()) = y;
  x.slice(y.size(),x.size()).fill(0);
}

template<int d> static inline void mpz_set(RawArray<mp_limb_t> x, const Exact<d>& y) {
  mpz_set(x,asarray(y.n));
}

// For use in construction-related templates
template<int d> static inline void mpz_set(RawArray<mp_limb_t,2> x, const Tuple<Vector<Exact<d+1>,1>,Exact<d>>& y) {
  assert(x.m==2);
  mpz_set(x[0],asarray(y.x.x.n));
  mpz_set(x[1],asarray(y.y.n));
}
template<int d> static inline void mpz_set(RawArray<mp_limb_t,2> x, const Tuple<Vector<Exact<d+1>,2>,Exact<d>>& y) {
  assert(x.m==3);
  mpz_set(x[0],asarray(y.x.x.n));
  mpz_set(x[1],asarray(y.x.y.n));
  mpz_set(x[2],asarray(y.y.n));
}
template<int d> static inline void mpz_set(RawArray<mp_limb_t,2> x, const Tuple<Vector<Exact<d+1>,3>,Exact<d>>& y) {
  assert(x.m==4);
  mpz_set(x[0],asarray(y.x.x.n));
  mpz_set(x[1],asarray(y.x.y.n));
  mpz_set(x[2],asarray(y.x.z.n));
  mpz_set(x[3],asarray(y.y.n));
}

// String conversion

using std::ostream;
GEODE_CORE_EXPORT string mpz_str(RawArray<const mp_limb_t> limbs, const bool hex=false);
GEODE_CORE_EXPORT string mpz_str(Subarray<const mp_limb_t,2> limbs, const bool hex=false);

template<int d> static inline ostream& operator<<(ostream& output, const Exact<d>& x) {
  return output << mpz_str(asarray(x.n));
}

// Overloads to make One work with perturbed_predicate_sqrt and friends

static inline int weak_sign(One) {
  return 1;
}

static inline int mpz_set(RawArray<mp_limb_t> x, One) {
  // We'll never reach here, since the "filter" step always succeeds for One
  GEODE_UNREACHABLE();
}

struct SmallShift {
  int s;
};
static inline SmallShift operator<<(One, const int s) {
  SmallShift r;
  r.s = s;
  return r;
}
template<class T> static inline T operator*(const T& x, const SmallShift s) {
  return x<<s.s;
}

RawArray<mp_limb_t> trim(RawArray<mp_limb_t> x);
RawArray<const mp_limb_t> trim(RawArray<const mp_limb_t> x);

template<int d> Exact<d> abs(Exact<d> e) { return is_negative(e) ? -e : e; }

}
