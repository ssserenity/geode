//#####################################################################
// Class Array<T>
//#####################################################################
//
// Array represents a flat, one-dimensional array.  The array buffer can be shared between multiple instances
// without copying, since it is managed by an arbitrary PyObject.  By default, the owning PyObject is a Buffer,
// but if the array originated from python it could be a numpy array or other native PyObject.
//
// The const semantics of Array are analogous to pointers: const Array<T> can be modified but not resized,
// and Array<const T> can be resized but not modified.  These semantics are a consequence of shareability:
// const Array<T> can also be modified by copying it into an Array<T> first.
//
// Note that Array always owns its own data; owner is allowed to be null only for empty arrays.  RawArray
// should be used in threaded code to avoid reference counting for thread safety and speed, and is returned
// by slice and similar functions by default.
//
//#####################################################################
#pragma once

#include <other/core/array/ArrayBase.h>
#include <other/core/array/RawArray.h>
#include <other/core/python/Buffer.h>
#include <other/core/python/exceptions.h>
#include <other/core/python/forward.h>
#include <other/core/utility/debug.h>
#include <other/core/utility/format.h>
#include <other/core/utility/config.h>
#include <other/core/utility/range.h>
#include <other/core/utility/using.h>
#include <other/core/vector/Vector.h>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
namespace other {

using std::swap;

template<class T> struct IsArray<Array<T>>:public mpl::true_{};

// Check whether T is an array type with a shareable buffer
template<class TArray> struct IsShareableArray:public mpl::false_{};
template<class TArray> struct IsShareableArray<const TArray>:public IsShareableArray<TArray>{};

// Array<T> is shareable
template<class T> struct IsShareableArray<Array<T>>:public mpl::true_{};

template<class T,int d> PyObject* to_python(const Array<T,d>& array) OTHER_EXPORT;
template<class T,int d> struct FromPython<Array<T,d>>{OTHER_EXPORT static Array<T,d> convert(PyObject* object);};

template<class T_>
class Array<T_,1> : public ArrayBase<T_,Array<T_>> {
  typedef T_ T;
public:
  typedef typename boost::remove_const<T>::type Element;
  static const bool is_const = boost::is_const<T>::value;
  typedef T& result_type;
  enum Workaround1 {dimension=1};
  enum Workaround2 {d=dimension};

  typedef ArrayBase<T,Array> Base;
  OTHER_USING(first,last,copy,same_array) 
private:
  friend class Array<Element>;
  friend class Array<const Element>;
  struct Unusable{};

  int m;
  int max_size_; // buffer size
  T* data_;
  PyObject* owner_; // python object that owns the buffer
public:

  Array()
    : m(0), max_size_(0), data_(0), owner_(0) {}

  explicit Array(const Vector<int,d> &sizes, const bool initialize=true)
    : m(sizes.x), max_size_(sizes.x) {
    assert(m>=0);
    Buffer* buffer = Buffer::new_<T>(m);
    data_ = (T*)buffer->data;
    owner_ = (PyObject*)buffer;
    if (initialize) {
      if (IsScalarVectorSpace<T>::value)
        memset(data_,0,m*sizeof(T));
      else
        for (int i=0;i<m;i++)
          const_cast<Element*>(data_)[i] = T();
    }
  }

  explicit Array(const int m, const bool initialize=true)
    : m(m), max_size_(m) {
    assert(m>=0);
    Buffer* buffer = Buffer::new_<T>(m);
    data_ = (T*)buffer->data;
    owner_ = (PyObject*)buffer;
    if (initialize) {
      if (IsScalarVectorSpace<T>::value)
        memset(data_,0,m*sizeof(T));
      else
        for (int i=0;i<m;i++)
          const_cast<Element*>(data_)[i] = T();
    }
  }

  Array(const Array& source)
    : m(source.m), max_size_(source.max_size_), data_(source.data_), owner_(source.owner_) {
    assert(owner_ || !data_);
    // Share a reference to the source buffer without copying it
    OTHER_XINCREF(owner_);
  }

  Array(typename mpl::if_c<is_const,const Array<Element>&,Unusable>::type source)
    : m(source.m), max_size_(source.max_size_), data_(source.data_), owner_(source.owner_) {
    assert(owner_ || !data_);
    // Share a reference to the source buffer without copying it
    OTHER_XINCREF(owner_);
  }

  template<class TArray>
  explicit Array(const TArray& source, typename boost::enable_if<IsShareableArray<TArray>,Unusable>::type unused=Unusable())
    : m(source.m), max_size_(source.max_size_), data_(source.data_), owner_(source.owner_) {
    assert(owner_ || !data_);
    // Share a reference to the source buffer without copying it
    STATIC_ASSERT_SAME(Element,typename TArray::Element);
    OTHER_XINCREF(owner_);
  }

  explicit Array(const NdArray<T>& array) {
    OTHER_ASSERT(array.rank()==1);
    m = max_size_=array.shape[0];
    data_ = array.data(); 
    owner_ = array.owner();
  }

  Array(const int m, T* data, PyObject* owner)
    : m(m), max_size_(m), data_(data), owner_(owner) {
    assert(owner_ || !data_);
    OTHER_XINCREF(owner_);
  }

  Array(const Vector<int,1>& counts, T* data, PyObject* owner)
    : m(counts.x), max_size_(m), data_(data), owner_(owner) {
    assert(owner_ || !data_);
    OTHER_XINCREF(owner_);
  }

  ~Array() {
    OTHER_XDECREF(owner_);
  }

  RawArray<T> raw() const { // Return a non-owning array for use in threaded code where reference counting is bad
    return RawArray<T>(m,data_);
  }

  int size() const {
    return m;
  }

  int total_size() const {
    return m;
  }

  Vector<int,1> sizes() const {
    return Vector<int,1>(m);
  }

  T& operator[](const int i) const {
    assert(unsigned(i)<unsigned(m));
    return data_[i];
  }

  T& operator()(const int i) const {
    assert(unsigned(i)<unsigned(m));
    return data_[i];
  }

  bool valid(const int i) const {
    return unsigned(i)<unsigned(m);
  }

  T* data() const {
    return data_;
  }

  PyObject* owner() const {
    OTHER_XINCREF(owner_);
    return owner_;
  }

  PyObject* borrow_owner() const {
    return owner_;
  }

  int max_size() const {
    return max_size_;
  }

  void clean_memory() {
    Array empty;
    swap(empty);
  }

  void clear() {
    m = 0;
  }

  void swap(Array& other) {
    std::swap(m,other.m);
    std::swap(max_size_,other.max_size_);
    std::swap(data_,other.data_);
    std::swap(owner_,other.owner_);
  }

  Array& operator=(const Array& source) {
    PyObject* owner_save = owner_;
    // Share a reference to the source buffer without copying it
    OTHER_XINCREF(source.owner_);
    owner_ = source.owner_;
    m = source.m;
    max_size_ = source.max_size_;
    data_ = source.data_;
    // Call decref last in case of side effects or this==&source
    OTHER_XDECREF(owner_save);
    return *this;
  }

  template<class TArray> typename boost::enable_if<IsShareableArray<TArray>,Array&>::type operator=(const TArray& source) {
    assert(source.owner_ || !source.data_);
    PyObject* owner_save = owner_;
    // Share a reference to the source buffer without copying it
    OTHER_XINCREF(source.owner_);
    owner_ = source.owner_;
    m = source.m;
    max_size_ = source.max_size_;
    data_ = source.data_;
    // Call decref last in case of side effects or this==&source
    OTHER_XDECREF(owner_save);
    return *this;
  }

  template<class TArray> void copy(const TArray& source) {
    // Copy data from source array even if it is shareable
    STATIC_ASSERT_SAME(T,typename TArray::Element);
    int source_m = source.size();
    if (max_size_<source_m)
      grow_buffer(source_m,false);
    if (!same_array(*this,source))
      for (int i=0;i<source_m;i++)
        data_[i] = source[i];
    m = source_m;
  }

  template<class TArray> void copy(const TArray& source) const {
    // Const, so no resizing allowed
    STATIC_ASSERT_SAME(T,typename TArray::Element);
    int source_m = source.size();
    assert(m==source_m);
    if (!same_array(*this,source))
      for (int i=0;i<source_m;i++)
        data_[i] = source[i];
  }

private:
  void grow_buffer(const int max_size_new, const bool copy_existing_elements=true) {
    if (max_size_>=max_size_new) return;
    Buffer* new_owner = Buffer::new_<T>(max_size_new);
    int m_ = m; // teach compiler that m is constant
    if (copy_existing_elements)
      for (int i=0;i<m_;i++)
        ((T*)new_owner->data)[i] = data_[i];
    OTHER_XDECREF(owner_);
    max_size_ = max_size_new;
    data_ = (T*)new_owner->data;
    owner_ = (PyObject*)new_owner;
  }
public:

  void preallocate(const int m_new, const bool copy_existing_elements=true) OTHER_ALWAYS_INLINE {
    if(max_size_<m_new)
      grow_buffer(other::max(4*max_size_/3+2,m_new),copy_existing_elements);
  }

  void resize(const int m_new, const bool initialize_new_elements=true, const bool copy_existing_elements=true) {
    preallocate(m_new,copy_existing_elements);
    if (initialize_new_elements && m_new>m) {
      if (IsScalarVectorSpace<T>::value)
        memset(data_+m,0,(m_new-m)*sizeof(T));
      else
        for (int i=m;i<m_new;i++) data_[i] = T();
    }
    m = m_new;
  }

  void exact_resize(const int m_new, const bool initialize_new_elements=true, const bool copy_existing_elements=true) { // Zero elbow room
    if (m==m_new) return;
    int m_end = other::min(m,m_new);
    if (max_size_!=m_new) {
      Buffer* new_owner = Buffer::new_<T>(m_new);
      if (copy_existing_elements)
        for (int i=0;i<m_end;i++)
          new_owner->data[i] = data_[i];
      OTHER_XDECREF(owner_);
      max_size_ = m_new;
      data_ = (T*)new_owner->data;
      owner_ = (PyObject*)new_owner;
    }
    if (initialize_new_elements && m_new>m_end) {
      if (IsScalarVectorSpace<T>::value)
        memset(data_+m_end,0,(m_new-m_end)*sizeof(T));
      else
        for (int i=m_end;i<m_new;i++)
          data_[i] = T();
    }
    m = m_new;
  }

  void compact() { // Note: if the buffer is shared, the memory will not be deallocated
    if (m<max_size_)
      exact_resize(m);
  }

  RawArray<T> reshape(int m_new) const {
    assert(m_new==m);
    return RawArray<T>(m_new,data());
  }

  RawArray<T,2> reshape(int m_new,int n_new) const {
    assert(m_new*n_new==m);
    return RawArray<T,2>(m_new,n_new,data());
  }

  const Array<T,2> reshape_own(int m_new,int n_new) const {
    assert(m_new*n_new==m);
    return Array<T,2>(m_new,n_new,data(),owner_);
  }

  const Array<T,2> reshape_own(Vector<int,2> new_sizes) const {
    return reshape_own(new_sizes.x,new_sizes.y);
  }

  RawArray<T,3> reshape(int m_new,int n_new,int mn_new) const {
    assert(m_new*n_new*mn_new==m);
    return RawArray<T,3>(m_new,n_new,mn_new,data());
  }

  const Array<T,3> reshape_own(int m_new,int n_new,int mn_new) const {
    assert(m_new*n_new*mn_new==m);
    return Array<T,3>(m_new,n_new,mn_new,data(),owner_);
  }

  const Array<T,3> reshape_own(Vector<int,3> new_sizes) const {
    return reshape_own(new_sizes.x,new_sizes.y,new_sizes.z);
  }

  int append(const T& element) OTHER_ALWAYS_INLINE {
    if (m<max_size_)
      data_[m++] = element;
    else {
      T save = element; // element could be reference into the current array
      preallocate(m+1);
      data_[m++] = save;
    }
    return m-1;
  }

  int append_assuming_enough_space(const T& element) OTHER_ALWAYS_INLINE {
    assert(m<max_size_);
    data_[m++] = element;
    return m-1;
  }

  template<class TArray> void append_elements(const TArray& append_array) {
    STATIC_ASSERT_SAME(T,typename TArray::Element);
    int append_m = append_array.size(),
        m_new = m+append_m;
    if (max_size_<m_new)
      grow_buffer(m_new);
    for (int i=0;i<append_m;i++)
      data_[m+i] = append_array[i];
    m = m_new;
  }

  void append_unique(const T& element) {
    if (!contains(element))
      append(element);
  }

  template<class TArray> void append_unique_elements(const TArray& append_array) {
    STATIC_ASSERT_SAME(T,typename TArray::Element);
    int append_m = append_array.size();
    for (int i=0;i<append_m;i++)
      append_unique(append_array(i));
  }

  void remove_index(const int index) { // Preserves ordering of remaining elements
    assert(unsigned(index)<unsigned(m));
    for (int i=index;i<m-1;i++)
      data_[i] = data_[i+1];
    m--;
  }

  void remove_index_lazy(const int index) { // Fill holes with last()
    assert(unsigned(index)<unsigned(m));
    data_[index] = data_[--m];
  }

  void insert(const T& element, const int index) {
    preallocate(m+1);
    m++;
    for (int i=m-1;i>index;i--)
      data_[i] = data_[i-1];
    data_[index] = element;
  }

  T& pop() { // Returns a temporarily valid reference (safe since ~T() is trivial)
    assert(m);
    return data_[--m];
  }

  Array<const T> pop_elements(const int count) { // return value shares ownership with original
    BOOST_MPL_ASSERT((boost::has_trivial_destructor<T>));
    assert(m-count>=0);
    m -= count;
    return Array<const T>(count,data_+m,owner_);
  }

  Array<Element>& const_cast_() {
    return *(Array<Element>*)this;
  }

  const Array<Element>& const_cast_() const {
    return *(const Array<Element>*)this;
  }

  const Array<const Element>& const_() const {
    return *(const Array<const Element>*)this;
  }

  RawArray<T> slice(int lo,int hi) const {
    assert(unsigned(lo)<=unsigned(hi) && unsigned(hi)<=unsigned(m));
    return RawArray<T>(hi-lo,data_+lo);
  }

  Array<T> slice_own(int lo,int hi) const {
    assert(unsigned(lo)<=unsigned(hi) && unsigned(hi)<=unsigned(m));
    return Array(hi-lo,data_+lo,owner_);
  }

  RawArray<T> slice(Range<int> range) const {
    return slice(range.lo,range.hi);
  }

  Array<T> slice_own(Range<int> range) const {
    return slice_own(range.lo,range.hi);
  }

  void zero() const {
    BOOST_MPL_ASSERT((IsScalarVectorSpace<T>));
    memset(data_,0,m*sizeof(T));
  }

  template<class T2> typename boost::enable_if<boost::is_same<T2,Element>,Array<T>>::type as() const {
    return *this;
  }

  template<class T2> typename boost::disable_if<boost::is_same<T2,Element>,Array<T2>>::type as() const {
    Array<T2> copy(m,false);
    for (int i=0;i<m;i++) copy[i] = data_[i];
    return copy;
  }
};

template<class T,int d> static inline const RawArray<T> as_array(Vector<T,d>& v) {
  return RawArray<T>(d,v.begin());
}

template<class T,int d> static inline const RawArray<const T> as_array(const Vector<T,d>& v) {
  return RawArray<const T>(d,v.begin());
}

template<class T,class A> static inline const RawArray<T> as_array(std::vector<T,A>& v) {
  return RawArray<T>(v.size(),&v[0]);
}

template<class T,class A> static inline const RawArray<const T> as_array(const std::vector<T,A>& v) {
  return RawArray<const T>(v.size(),&v[0]);
}

}
namespace std{
template<class T,int d> void swap(other::Array<T,d>& array1, other::Array<T,d>& array2) {
  array1.swap(array2);
}
}
