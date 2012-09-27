//#####################################################################
// Class LinearFiniteVolume
//#####################################################################
#pragma once

#include <other/core/force/Force.h>
#include <other/core/force/StrainMeasure.h>
namespace other{

template<class TV,int d>
class LinearFiniteVolume:public Force<TV>
{
  typedef typename TV::Scalar T;
  enum {m=TV::m};
public:
  typedef Force<TV> Base;
  OTHER_DECLARE_TYPE

  const Array<const Vector<int,d+1> > elements;
  T youngs_modulus;
  T poissons_ratio;
  T rayleigh_coefficient;
  const int nodes;
  const T density;
  const Array<const Matrix<T,d,m> > Dm_inverse;
private:
  Array<TV> normals;
  Array<T> Bm_scales; // Bm[t] = Bm_scales[t]*Dm_inverse[t].transposed()
  Array<const TV> X;

protected:
  LinearFiniteVolume(Array<const Vector<int,d+1> > elements,Array<const TV> X,const T density,const T youngs_modulus,const T poissons_ratio,const T rayleigh_coefficient);
public:
  virtual ~LinearFiniteVolume();

  Matrix<T,m,d> Ds(RawArray<const TV> X,const int simplex) const {
    return StrainMeasure<TV,d>::Ds(X,elements[simplex]);
  }

  Vector<T,2> mu_lambda() const;

  void update_position(Array<const TV> X,bool definite);
  T elastic_energy() const;
  void add_elastic_force(RawArray<TV> F) const;
  void add_elastic_differential(RawArray<TV> dF,RawArray<const TV> dX) const;
  void add_elastic_gradient_block_diagonal(RawArray<SymmetricMatrix<T,m> > dFdX) const;
  T damping_energy(RawArray<const TV> V) const;
  void add_damping_force(RawArray<TV> F,RawArray<const TV> V) const;
  void add_frequency_squared(RawArray<T> frequency_squared) const;
  T strain_rate(RawArray<const TV> V) const;

  void structure(SolidMatrixStructure& structure) const;
  void add_elastic_gradient(SolidMatrix<TV>& matrix) const;
  void add_damping_gradient(SolidMatrix<TV>& matrix) const;
private:
  void add_differential_helper(RawArray<TV> dF,RawArray<const TV> dX,T scale) const;
};

}