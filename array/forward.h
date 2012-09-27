//#####################################################################
// Header Arrays/forward
//#####################################################################
#pragma once

#include <other/core/python/config.h> // Must be included before other headers
#include <boost/mpl/bool.hpp>
namespace other {

namespace mpl = boost::mpl;

template<class T,class TArray> class ArrayBase;
template<class T,class TArray,class... Args> class ArrayExpression;
template<class T,class TArray> class ArrayNdBase;

template<class T,int d> class Vector;
template<class T,int d=1> class Array;
template<class T,int d=1> class RawArray;
template<class T,int d=1> class Subarray;
template<class T> class NdArray;
template<class TArray,class TIndices=RawArray<const int> > class IndirectArray;
class IdentityMap;
template<class T> class ConstantMap;
template<class T> class NestedArray;
template<class T> class NdArray;

template<class T,class Id> class Field;
template<class T,class Id> class RawField;
template<class Iter> struct Range;

template<class TArray,class TProjector> class ProjectedArray;
template<class TStruct,class TField,TField TStruct::* field> struct FieldProjector;
struct IndexProjector;
struct PointerProjector;

template<class TArray,class TNew> struct Rebind;
template<class T,int d,class TNew> struct Rebind<Array<T,d>,TNew>{typedef Array<TNew,d> type;};

template<class TArray,class Enabler=void> struct IsArray:mpl::false_{};
template<class TArray> struct IsArray<const TArray>:IsArray<TArray>{};

template<class TArray> struct IsShareable:mpl::false_{};
template<class T> struct IsShareable<const T>:IsShareable<T>{};
template<class T,int d> struct IsShareable<Array<T,d> >:mpl::true_{};
template<class T> struct IsShareable<NdArray<T> >:mpl::true_{};

template<class TA> struct IsContiguousArray:mpl::false_{};
template<class TA> struct IsContiguousArray<const TA>:IsContiguousArray<TA>{};
template<class T,int d> struct IsContiguousArray<Array<T,d> >:mpl::true_{};
template<class T,int d> struct IsContiguousArray<RawArray<T,d> >:mpl::true_{};

}
namespace std {
template<class T,class A> class vector;
}