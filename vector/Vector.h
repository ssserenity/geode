//#####################################################################
// Class Vector
//#####################################################################
#pragma once

#include <other/core/vector/Vector0d.h>
#include <other/core/vector/Vector1d.h>
#include <other/core/vector/Vector2d.h>
#include <other/core/vector/Vector3d.h>
#include <other/core/vector/Vector4d.h>
#include <other/core/math/clamp.h>
#include <other/core/math/inverse.h>
#include <other/core/math/max.h>
#include <other/core/math/min.h>
#include <other/core/math/sqr.h>
#include <other/core/python/forward.h>
#include <other/core/python/repr.h>
#include <other/core/python/to_python.h>
#include <boost/type_traits/common_type.hpp>
#include <other/core/python/config.h>
#include <cmath>
namespace other {

using ::std::abs;
using ::std::floor;
using ::std::ceil;
using ::std::sqrt;
using ::std::exp;
using ::std::sin;
using ::std::cos;
using ::std::pow;
using boost::common_type;

template<class TArray,class TIndices> class IndirectArray;

template<class T,int d> PyObject* to_python(const Vector<T,d>& vector) OTHER_EXPORT;
template<class T,int d> struct FromPython<Vector<T,d> >{OTHER_EXPORT static Vector<T,d> convert(PyObject* object);};

template<class T,int d>
class Vector
{
    BOOST_STATIC_ASSERT(d>3);
    struct Unusable{};
public:
    template<class T2> struct Rebind{typedef Vector<T2,d> type;};
    typedef typename mpl::if_<IsScalar<T>,T,Unusable>::type Scalar;
    typedef T Element;
    typedef T value_type; // for stl
    typedef T* iterator; // for stl
    typedef const T* const_iterator; // for stl
    template<class> class result;
    template<class V> class result<V(int)>:public mpl::if_<boost::is_const<V>,const T&,T&>{};
    enum Workaround1 {dimension=d};
    enum Workaround2 {m=d};
    static const bool is_const=false;

    T array[d];

    Vector()
    {
        for(int i=0;i<d;i++) array[i]=T();
    }

    Vector(const T& x0,const T& x1,const T& x2,const T& x3)
    {
        BOOST_STATIC_ASSERT(d==4);array[0]=x0;array[1]=x1;array[2]=x2;array[3]=x3;
    }

    Vector(const T& x0,const T& x1,const T& x2,const T& x3,const T& x4)
    {
        BOOST_STATIC_ASSERT(d==5);array[0]=x0;array[1]=x1;array[2]=x2;array[3]=x3;array[4]=x4;
    }

    Vector(const T& x0,const T& x1,const T& x2,const T& x3,const T& x4,const T& x5)
    {
        BOOST_STATIC_ASSERT(d==6);array[0]=x0;array[1]=x1;array[2]=x2;array[3]=x3;array[4]=x4;array[5]=x5;
    }

    template<class T2,int d2>
    explicit Vector(const Vector<T2,d2>& v)
    {
        BOOST_STATIC_ASSERT(d2<=d);
        for(int i=0;i<d2;i++) array[i]=(T)v[i];
        for(int i=d2;i<d;i++) array[i]=T();
    }

    template<class TVector>
    explicit Vector(const TVector& v)
    {
        BOOST_STATIC_ASSERT((boost::is_same<T,typename TVector::Element>::value && TVector::m==d));
        for(int i=0;i<d;i++) array[i]=v[i];
    }

    Vector(const Vector& v)
    {
        for(int i=0;i<d;i++) array[i]=v.array[i];
    }

    template<int n>
    Vector(const Vector<T,n>& v1,const Vector<T,d-n>& v2)
    {
        for(int i=0;i<n;i++) (*this)(i)=v1(i);for(int i=n;i<d;i++) (*this)(i)=v2(i-n);
    }

    template<class TVector> typename boost::enable_if<mpl::and_<boost::is_same<T,typename TVector::Element>,mpl::bool_<TVector::m==d> >,Vector&>::type
    operator=(const TVector& v)
    {
        for(int i=0;i<d;i++) array[i]=v[i];return *this;
    }

    Vector& operator=(const Vector& v)
    {
        for(int i=0;i<d;i++) array[i]=v.array[i];return *this;
    }

    int size() const
    {return m;}

    const T& operator[](const int i) const
    {assert(unsigned(i)<d);return array[i];}

    T& operator[](const int i)
    {assert(unsigned(i)<d);return array[i];}

    T* data()
    {return array;}

    const T* data() const
    {return array;}

    template<class TIndices>
    IndirectArray<Vector,TIndices&> subset(const TIndices& indices)
    {return IndirectArray<Vector,TIndices&>(*this,indices);}

    template<class TIndices>
    IndirectArray<const Vector,TIndices&> subset(const TIndices& indices) const
    {return IndirectArray<const Vector,TIndices&>(*this,indices);}

    bool operator==(const Vector& v) const
    {for(int i=0;i<d;i++) if(array[i]!=v.array[i]) return false;return true;}

    bool operator!=(const Vector& v) const
    {return !((*this)==v);}

    Vector operator-() const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=-array[i];return r;}

    Vector operator+(const T& a) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]+a;return r;}

    Vector& operator+=(const T& a)
    {for(int i=0;i<d;i++) array[i]+=a;return *this;}

    Vector operator-(const T& a) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]-a;return r;}

    Vector& operator-=(const T& a)
    {for(int i=0;i<d;i++) array[i]-=a;return *this;}

    Vector operator+(const Vector& v) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]+v.array[i];return r;}

    Vector& operator+=(const Vector& v)
    {for(int i=0;i<d;i++) array[i]+=v.array[i];return *this;}

    Vector operator-(const Vector& v) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]-v.array[i];return r;}

    Vector& operator-=(const Vector& v)
    {for(int i=0;i<d;i++) array[i]-=v.array[i];return *this;}

    Vector operator*(const T& a) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]*a;return r;}

    Vector& operator*=(const T& a)
    {for(int i=0;i<d;i++) array[i]*=a;return *this;}

    Vector operator>>(const int a)
    {Vector r; for(int i = 0; i < d; ++i) r.array[i] = array[i] >> a; return r;}

    Vector operator<<(const int a)
    {Vector r; for(int i = 0; i < d; ++i) r.array[i] = array[i] << a; return r;}

    Vector operator/(const T& a) const
    {return *this*(1/a);}

    Vector& operator/=(const T& a)
    {return *this*=1/a;}

    Vector operator*(const Vector& v) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]*v.array[i];return r;}

    Vector& operator*=(const Vector& v)
    {for(int i=0;i<d;i++) array[i]*=v.array[i];return *this;}

    Vector operator/(const Vector& v) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]/v.array[i];return r;}

    Vector& operator/=(const Vector& v)
    {for(int i=0;i<d;i++) array[i]/=v.array[i];return *this;}

    Vector operator*(const IntInverse<T> a) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]*a;return r;}

    Vector operator&(const T& a) const
    {Vector r;for(int i=0;i<d;i++) r.array[i]=array[i]&a;return r;}

    T sqr_magnitude() const
    {T r=0;for(int i=0;i<d;i++) r+=sqr(array[i]);return r;}

    T magnitude() const
    {return sqrt(sqr_magnitude());}

    T normalize()
    {T magnitude=magnitude();if(magnitude) *this*=1/magnitude;else *this=axis_vector(0);return magnitude;}

    Vector normalized() const
    {T magnitude=magnitude();if(magnitude) return *this*(1/magnitude);else return axis_vector(0);}

    T min() const
    {T r=array[0];for(int i=1;i<d;i++) r=other::min(r,array[i]);return r;}

    T max() const
    {T r=array[0];for(int i=1;i<d;i++) r=other::max(r,array[i]);return r;}

    bool elements_equal() const
    {bool equal=true;for(int i=0;i<d;i++) equal&=(array[i]==array[0]);return equal;}

    static Vector componentwise_min(const Vector& v1,const Vector& v2)
    {Vector r;for(int i=0;i<d;i++) r.array[i]=other::min(v1.array[i],v2.array[i]);return r;}

    static Vector componentwise_max(const Vector& v1,const Vector& v2)
    {Vector r;for(int i=0;i<d;i++) r.array[i]=other::max(v1.array[i],v2.array[i]);return r;}

    Vector projected_on_unit_direction(const Vector& direction) const
    {return dot(*this,direction)*direction;}

    Vector projected(const Vector& direction) const // un-normalized direction
    {return dot(*this,direction)/direction.sqr_magnitude()*direction;}

    void project_on_unit_direction(const Vector& direction)
    {*this=dot(*this,direction)*direction;}

    void project(const Vector& direction) // un-normalized direction
    {*this=dot(*this,direction)/direction.sqr_magnitude()*direction;}

    Vector projected_orthogonal_to_unit_direction(const Vector& direction) const
    {return *this-dot(*this,direction)*direction;}

    void project_orthogonal_to_unit_direction(const Vector& direction)
    {*this-=dot(*this,direction)*direction;}

    T sum() const
    {T r=array[0];for(int i=1;i<d;i++) r+=array[i];return r;}

    T average() const
    {return T(1./d)*sum();}

    T product() const
    {T r=array[0];for(int i=1;i<d;i++) r*=array[i];return r;}

    const Vector& column_sum() const
    {return *this;}

    int number_true() const
    {STATIC_ASSERT_SAME(T,bool);int count=0;for(int i=0;i<d;i++)if(array[i]) count++;return count;}

    static Vector axis_vector(const int axis)
    {Vector r;r(axis)=(T)1;return r;}

    static Vector ones()
    {Vector r;for(int i=0;i<d;i++) r.array[i]=(T)1;return r;}

    // shifts vector (wrapped) such that element a is first
    Vector<T,d> roll(int a) const {
      Vector<T,d> v;
      for (int i = 0; i < d; ++i) {
        v[i] = (*this)[(i+a) % d];
      }
      return v;
    }


    void fill(const T& constant)
    {for(int i=0;i<d;i++) array[i]=constant;}

    void get(T& element0,T& element1,T& element2,T& element3) const
    {BOOST_STATIC_ASSERT(d==4);element0=array[0];element1=array[1];element2=array[2];element3=array[3];}

    void get(T& element0,T& element1,T& element2,T& element3,T& element4) const
    {BOOST_STATIC_ASSERT(d==5);element0=array[0];element1=array[1];element2=array[2];element3=array[3];element4=array[4];}

    void get(T& element0,T& element1,T& element2,T& element3,T& element4,T& element5) const
    {BOOST_STATIC_ASSERT(d==6);element0=array[0];element1=array[1];element2=array[2];element3=array[3];element4=array[4];element5=array[5];}

    void get(T& element0,T& element1,T& element2,T& element3,T& element4,T& element5,T& element6) const
    {BOOST_STATIC_ASSERT(d==7);element0=array[0];element1=array[1];element2=array[2];element3=array[3];element4=array[4];element5=array[5];element6=array[6];}

    void get(T& element0,T& element1,T& element2,T& element3,T& element4,T& element5,T& element6,T& element7) const
    {BOOST_STATIC_ASSERT(d==8);element0=array[0];element1=array[1];element2=array[2];element3=array[3];element4=array[4];element5=array[5];element6=array[6];element7=array[7];}

    void set(const T& element0,const T& element1,const T& element2,const T& element3)
    {BOOST_STATIC_ASSERT(d==4);array[0]=element0;array[1]=element1;array[2]=element2;array[3]=element3;}

    void set(const T& element0,const T& element1,const T& element2,const T& element3,const T& element4)
    {BOOST_STATIC_ASSERT(d==5);array[0]=element0;array[1]=element1;array[2]=element2;array[3]=element3;array[4]=element4;}

    void set(const T& element0,const T& element1,const T& element2,const T& element3,const T& element4,const T& element5)
    {BOOST_STATIC_ASSERT(d==6);array[0]=element0;array[1]=element1;array[2]=element2;array[3]=element3;array[4]=element4;array[5]=element5;}

    void set(const T& element0,const T& element1,const T& element2,const T& element3,const T& element4,const T& element5,const T& element6)
    {BOOST_STATIC_ASSERT(d==7);array[0]=element0;array[1]=element1;array[2]=element2;array[3]=element3;array[4]=element4;array[5]=element5;array[6]=element6;}

    void set(const T& element0,const T& element1,const T& element2,const T& element3,const T& element4,const T& element5,const T& element6,const T& element7)
    {BOOST_STATIC_ASSERT(d==8);array[0]=element0;array[1]=element1;array[2]=element2;array[3]=element3;array[4]=element4;array[5]=element5;array[6]=element6;array[7]=element7;}

    template<class TFunction>
    static Vector map(const TFunction& f,const Vector& v)
    {Vector r;for(int i=0;i<d;i++) r.array[i]=f(v.array[i]);return r;}

    int find(const T& element) const
    {for(int i=0;i<d;i++) if(array[i]==element) return i;return -1;}

    bool contains(const T& element) const
    {for(int i=0;i<d;i++) if(array[i]==element) return true;return false;}

    template<class TArray>
    bool contains_all(const TArray& elements) const
    {STATIC_ASSERT_SAME(typename TArray::Element,T);
    for(int i=0;i<elements.size();i++) if(!contains(elements(i))) return false;
    return true;}

    template<class TArray>
    bool contains_any(const TArray& elements) const
    {STATIC_ASSERT_SAME(typename TArray::Element,T);
    for(int i=0;i<elements.size();i++) if(contains(elements(i))) return true;
    return false;}

    Vector<T,d-1> remove_index(const int index) const
    {assert(unsigned(index)<d);Vector<T,d-1> r;for(int i=0;i<d-1;i++) r[i]=(*this)[i+(i>=index)];return r;}

    Vector<T,d+1> insert(const T& element,const int index) const
    {Vector<T,d+1> r;r[index]=element;for(int i=0;i<d;i++) r[i+(i>=index)]=(*this)[i];return r;}

    Vector<T,d+1> append(const T& element) const
    {Vector<T,d+1> r;for(int i=0;i<d;i++) r[i]=(*this)[i];r[d]=element;return r;}

    template<int d2> Vector<T,d+d2> append_elements(const Vector<T,d2>& elements)
    {Vector<T,d+d2> r;
    for(int i=0;i<d;i++) r[i]=(*this)[i];
    for(int i=0;i<d2;i++) r[i+d]=elements[i];
    return r;}

    Vector<T,4> sorted() const
    {BOOST_STATIC_ASSERT(d==4);Vector<T,4> r(*this);small_sort(r[0],r[1],r[2],r[3]);return r;}

    Vector reversed() const
    {Vector r;for(int i=0;i<d;i++) r.array[d-1-i]=array[i];return r;}

    template<int d1,int d2> Vector<T,d2-d1> slice() const
    {BOOST_STATIC_ASSERT((mpl::and_<mpl::less_equal<mpl::int_<0>,mpl::int_<d1> >,mpl::less_equal<mpl::int_<d2>,mpl::int_<d> > >::value));
    Vector<T,d2-d1> r;for(int i=d1;i<d2;i++) r[i-d1]=(*this)[i];return r;}

    template<int n> void split(Vector<T,n>& v1,Vector<T,d-n>& v2) const
    {for(int i=0;i<n;i++) v1(i)=(*this)(i);
    for(int i=n;i<d;i++) v2(i-n)=(*this)(i);}

    T* begin() // for stl
    {return array;}

    const T* begin() const // for stl
    {return array;}

    T* end() // for stl
    {return array+d;}

    const T* end() const // for stl
    {return array+d;}

//#####################################################################
};

typedef Vector<real, 2> Vector2;
typedef Vector<real, 3> Vector3;
typedef Vector<real, 4> Vector4;

//#####################################################################
// Miscellaneous free operators and functions
//#####################################################################
template<class T,int d> inline Vector<T,d>
operator+(const T& a,const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=a+v.array[i];return r;}

template<class T,int d> inline Vector<T,d>
operator-(const T& a,const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=a-v.array[i];return r;}

template<class T,int d> inline Vector<T,d>
operator*(const T& a,const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=a*v.array[i];return r;}

template<class T,int d> inline Vector<T,d>
operator/(const T& a,const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=a/v.array[i];return r;}

template<class T,int d> inline Vector<T,d>
abs(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=abs(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
floor(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=floor(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
ceil(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=ceil(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
exp(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=exp(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
sin(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=sin(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
cos(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=cos(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
sqrt(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=sqrt(v.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
inverse(const Vector<T,d>& v)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=1/v.array[i];return r;}

template<class T,int d>
inline Vector<T,d> wrap(const Vector<T,d>& value,const Vector<T,d>& lower,const Vector<T,d>& upper)
{Vector<T,d> result;for(int i=0;i<d;i++) result(i)=wrap(value(i),lower(i),upper(i));return result;}

template<class T,int d>
inline bool isfinite(const Vector<T,d>& v)
{for(int i=0;i<d;i++) if(!isfinite(v[i])) return false;return true;}

//#####################################################################
// Functions clamp, clamp_min, clamp_max, in_bounds
//#####################################################################
template<class T,int d> inline Vector<T,d>
clamp(const Vector<T,d>& v,const Vector<T,d>& vmin,const Vector<T,d>& vmax)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=clamp(v.array[i],vmin.array[i],vmax.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
clamp(const Vector<T,d>& v,const T& min,const T& max)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=clamp(v.array[i],min,max);return r;}

template<class T,int d> inline Vector<T,d>
clamp_min(const Vector<T,d>& v,const Vector<T,d>& vmin)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=clamp_min(v.array[i],vmin.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
clamp_min(const Vector<T,d>& v,const T& min)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=clamp_min(v.array[i],min);return r;}

template<class T,int d> inline Vector<T,d>
clamp_max(const Vector<T,d>& v,const Vector<T,d>& vmax)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=clamp_max(v.array[i],vmax.array[i]);return r;}

template<class T,int d> inline Vector<T,d>
clamp_max(const Vector<T,d>& v,const T& max)
{Vector<T,d> r;for(int i=0;i<d;i++) r.array[i]=clamp_max(v.array[i],max);return r;}

template<class T,int d> inline Vector<T,d>
in_bounds(const Vector<T,d>& v,const Vector<T,d>& vmin,const Vector<T,d>& vmax)
{for(int i=0;i<d;i++) if(!in_bounds(v.array[i],vmin.array[i],vmax.array[i])) return false;
return true;}

//#####################################################################
// Stream input and output
//#####################################################################
template<class T,int d> inline std::istream&
operator>>(std::istream& input,Vector<T,d>& v)
{input>>expect('[');if(d) input>>v[0];for(int i=1;i<d;i++) input>>expect(',')>>v[i];return input>>expect(']');}

template<class T,int d> inline std::ostream&
operator<<(std::ostream& output,const Vector<T,d>& v)
{output<<'[';if(d) output<<v[0];for(int i=1;i<d;i++) output<<","<<v[i];output<<']';return output;}

template<int d> inline std::ostream&
operator<<(std::ostream& output,const Vector<unsigned char,d>&v)
{output<<'[';if(d) output<<(int)v[0];for(int i=1;i<d;i++) output<<","<<(int)v[i];output<<']';return output;}

template<class T> string
tuple_repr(const Vector<T,1>& v)
{return format("(%s,)",repr(v[0]));}

template<class T> string
tuple_repr(const Vector<T,2>& v)
{return format("(%s,%s)",repr(v[0]),repr(v[1]));}

template<class T> string
tuple_repr(const Vector<T,3>& v)
{return format("(%s,%s,%s)",repr(v[0]),repr(v[1]),repr(v[2]));}

template<class T> string
tuple_repr(const Vector<T,4>& v)
{return format("(%s,%s,%s,%s)",repr(v[0]),repr(v[1]),repr(v[2]),repr(v[3]));}

//#####################################################################
// Vector construction
//#####################################################################
template<class T,class... Args> static inline auto vec(const Args&... args)
  -> Vector<T,sizeof...(Args)> {
  return Vector<T,sizeof...(Args)>(args...);
}

template<class... Args> static inline auto vec(const Args&... args)
  -> Vector<typename common_type<Args...>::type,sizeof...(Args)> {
  return Vector<typename common_type<Args...>::type,sizeof...(Args)>(args...);
}

//#####################################################################
}