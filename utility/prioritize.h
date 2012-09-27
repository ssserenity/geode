#pragma once

namespace other {

template<class A, class P = double>
struct Prioritize {
  A a;
  P p;
  
  Prioritize(A const &a, P const &p)
    : a(a), p(p) {}
  
  bool operator<(const Prioritize<A,P>& b) const {
    return p < b.p;
  }

  bool operator>(const Prioritize<A,P>& b) const {
    return p > b.p;
  }
};

template<class A, class B>
inline Prioritize<A,B> prioritize(const A& a, const B& b) {
  return Prioritize<A,B>(a, b);
}  

}