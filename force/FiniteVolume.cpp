//#####################################################################
#include <other/core/force/FiniteVolume.h>
#include <other/core/force/AnisotropicConstitutiveModel.h>
#include <other/core/force/IsotropicConstitutiveModel.h>
#include <other/core/force/DiagonalizedStressDerivative.h>
#include <other/core/force/DiagonalizedIsotropicStressDerivative.h>
#include <other/core/force/PlasticityModel.h>
#include <other/core/force/StrainMeasure.h>
#include <other/core/structure/Hashtable.h>
#include <other/core/math/Factorial.h>
#include <other/core/python/Class.h>
#include <other/core/vector/DiagonalMatrix.h>
#include <other/core/vector/Matrix.h>
#include <other/core/vector/SolidMatrix.h>
#include <other/core/vector/SymmetricMatrix.h>
#include <other/core/vector/UpperTriangularMatrix.h>
#include <other/core/utility/Log.h>
namespace other{

using Log::cout;
using std::endl;

typedef real T;
template<> OTHER_DEFINE_TYPE(FiniteVolume<Vector<T,2>,2>)
template<> OTHER_DEFINE_TYPE(FiniteVolume<Vector<T,3>,2>)
template<> OTHER_DEFINE_TYPE(FiniteVolume<Vector<T,3>,3>)

template<class TV,int d> FiniteVolume<TV,d>::
FiniteVolume(Ref<StrainMeasure<TV,d> > strain,T density,Ref<ConstitutiveModel<T,d> > model,Ptr<PlasticityModel<T,d> > plasticity)
    :strain(strain),density(density),model(model),plasticity(plasticity)
{
    Be_scales.resize(strain->elements.size(),false,false);
    for(int t=0;t<Be_scales.size();t++)
        Be_scales[t] = -(T)1/Factorial<d>::value/strain->Dm_inverse[t].determinant();
    isotropic = dynamic_cast<IsotropicConstitutiveModel<T,d>*>(&*model);
    anisotropic = dynamic_cast<AnisotropicConstitutiveModel<T,d>*>(&*model);
}

template<class TV,int d> FiniteVolume<TV,d>::
~FiniteVolume()
{}

template<class TV,int d> void FiniteVolume<TV,d>::
structure(SolidMatrixStructure& structure) const
{
    for(int t=0;t<strain->elements.size();t++){
        Vector<int,d+1>  nodes = strain->elements[t];
        for(int i=0;i<nodes.size();i++) for(int j=i+1;j<nodes.size();j++)
            structure.add_entry(i,j);}
}

template<int d,int m> static inline typename boost::enable_if_c<m==d,const Matrix<T,d,d>&>::type
In_Plane(const Matrix<T,m>& U)
{
    return U;
}

template<int d> static inline typename boost::enable_if_c<d==2,Matrix<T,3,2> >::type
In_Plane(const Matrix<T,3>& U)
{
    return Matrix<T,3,2>(U.column(0),U.column(1));
}

template<class TV,int d> void FiniteVolume<TV,d>::
update_position(Array<const TV> X,bool definite_)
{
    definite = definite_;
    stress_derivatives_valid = false;
    U.resize(strain->elements.size(),false,false);
    De_inverse_hat.resize(strain->elements.size(),false,false);
    Fe_hat.resize(strain->elements.size(),false,false);
    if(anisotropic)
        V.resize(strain->elements.size(),false,false);
    for(int t=0;t<strain->elements.size();t++){
        Matrix<T,d> V_;
        if(plasticity){
            Matrix<T,m,d> F = strain->F(X,t);
            (F*plasticity->Fp_inverse(t)).fast_singular_value_decomposition(U[t],Fe_hat[t],V_);
            DiagonalMatrix<T,d> Fe_project_hat;
            if(plasticity->project_Fe(Fe_hat[t],Fe_project_hat)){
                plasticity->project_Fp(t,Fe_project_hat.inverse()*In_Plane<d>(U[t]).transpose_times(F));
                (F*plasticity->Fp_inverse[t]).fast_singular_value_decomposition(U[t],Fe_hat[t],V_);}
            De_inverse_hat[t] = strain->Dm_inverse[t]*plasticity->Fp_inverse[t]*V_;
            Be_scales[t] = -(T)1/Factorial<d>::value/De_inverse_hat[t].determinant();}
        else{
            strain->F(X,t).fast_singular_value_decomposition(U[t],Fe_hat[t],V_);
            De_inverse_hat[t] = strain->Dm_inverse[t]*V_;}
        if(anisotropic) anisotropic->update_position(Fe_hat[t],V_,t);
        else isotropic->update_position(Fe_hat[t],t);
        if(anisotropic) V[t] = V_;}
}

template<class TV,int d> typename TV::Scalar FiniteVolume<TV,d>::
elastic_energy() const
{
    T energy=0;
    if(anisotropic)
        for(int t=0;t<strain->elements.size();t++)
            energy -= Be_scales[t]*anisotropic->elastic_energy(Fe_hat[t],V[t],t);
    else
        for(int t=0;t<strain->elements.size();t++)
            energy -= Be_scales[t]*isotropic->elastic_energy(Fe_hat[t],t);
    return energy;
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_elastic_force(RawArray<TV> F) const
{
    if(anisotropic)
        for(int t=0;t<strain->elements.size();t++){
            Matrix<T,m,d> forces = In_Plane<d>(U[t])*anisotropic->P_From_Strain(Fe_hat[t],V[t],Be_scales[t],t).times_transpose(De_inverse_hat[t]);
            strain->distribute_force(F,t,forces);}
    else
        for(int t=0;t<strain->elements.size();t++){
            Matrix<T,m,d> forces = In_Plane<d>(U[t])*isotropic->P_From_Strain(Fe_hat[t],Be_scales[t],t).times_transpose(De_inverse_hat[t]);
            strain->distribute_force(F,t,forces);}
}

template<int m,int d> static inline typename boost::enable_if_c<m==d,const DiagonalizedIsotropicStressDerivative<T,m>&>::type
add_out_of_plane(const IsotropicConstitutiveModel<T,d>& model,const DiagonalMatrix<T,d>& F_hat,const DiagonalizedIsotropicStressDerivative<T,d>& in_plane,int t)
{
    return in_plane;
}

template<int m> static inline typename boost::enable_if_c<m==3,DiagonalizedIsotropicStressDerivative<T,3,2> >::type
add_out_of_plane(const IsotropicConstitutiveModel<T,2>& model,const DiagonalMatrix<T,2>& F_hat,const DiagonalizedIsotropicStressDerivative<T,2>& in_plane,int t)
{
    DiagonalizedIsotropicStressDerivative<T,3,2> A;
    A.A = in_plane;
    DiagonalMatrix<T,2> P = model.P_From_Strain(F_hat,1,t), F_clamp = model.clamp_f(F_hat);
    A.x2020 = P.x00/F_clamp.x00;
    A.x2121 = P.x11/F_clamp.x11;
    return A;
}

template<class TV,int d> void FiniteVolume<TV,d>::
update_stress_derivatives() const
{
    if(stress_derivatives_valid) return;
    OTHER_ASSERT(isotropic || TV::m==d); // codimension zero only for anisotropic for now
    if(anisotropic && !anisotropic->use_isotropic_stress_derivative()){
        dP_dFe.resize(strain->elements.size(),false,false);
        for(int t=0;t<strain->elements.size();t++){
            dP_dFe[t] = anisotropic->stress_derivative(Fe_hat[t],V[t],t);
            if(definite) dP_dFe[t].enforce_definiteness();}}
    else{
        dPi_dFe.resize(strain->elements.size(),false,false);
        for(int t=0;t<strain->elements.size();t++){
            dPi_dFe[t] = add_out_of_plane<m>(*isotropic,Fe_hat[t],model->isotropic_stress_derivative(Fe_hat[t],t),t);
            if(definite) dPi_dFe[t].enforce_definiteness();}}
    stress_derivatives_valid = true;
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_elastic_differential(RawArray<TV> dF,RawArray<const TV> dX) const
{
    update_stress_derivatives();
    if(anisotropic && !anisotropic->use_isotropic_stress_derivative())
        for(int t=0;t<strain->elements.size();t++){
            Matrix<T,m,d> dDs = strain->Ds(dX,t), Up = In_Plane<d>(U[t]),
                dG = Up*(Be_scales[t]*dP_dFe[t].differential(Up.transpose_times(dDs)*De_inverse_hat[t]).times_transpose(De_inverse_hat[t]));
            strain->distribute_force(dF,t,dG);}
    else
        for(int t=0;t<strain->elements.size();t++){
            Matrix<T,m,d> dDs = strain->Ds(dX,t),
                dG = U[t]*(Be_scales[t]*dPi_dFe[t].differential(U[t].transpose_times(dDs)*De_inverse_hat[t]).times_transpose(De_inverse_hat[t]));
            strain->distribute_force(dF,t,dG);}
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_elastic_gradient_block_diagonal(RawArray<SymmetricMatrix<T,m> > dFdX) const
{
    update_stress_derivatives();
    if(anisotropic && !anisotropic->use_isotropic_stress_derivative())
        OTHER_NOT_IMPLEMENTED();
    else{
        Matrix<T,m> dGdD[d][d];
        for(int t=0;t<strain->elements.size();t++){
            for(int i=0;i<d;i++) for(int j=0;j<m;j++){
                Matrix<T,m,d> dDs;
                dDs(j,i) = 1;
                Matrix<T,m,d> dG = U[t]*(Be_scales[t]*dPi_dFe[t].differential(U[t].transpose_times(dDs)*De_inverse_hat[t]).times_transpose(De_inverse_hat[t]));
                for(int k=0;k<d;k++)
                    dGdD[k][i].set_column(j,dG.column(k));}
            Vector<int,d+1> nodes = strain->elements[t];
            for(int i=0;i<d;i++)
                dFdX[nodes[i+1]] += assume_symmetric(dGdD[i][i]);
            SymmetricMatrix<T,m> sum; 
            for(int i=0;i<d;i++) for(int j=0;j<d;j++)
                sum += assume_symmetric(dGdD[i][j]);
            dFdX[nodes[0]] += sum;}}
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_elastic_gradient(SolidMatrix<TV>& matrix) const
{
    update_stress_derivatives();
    if(anisotropic && !anisotropic->use_isotropic_stress_derivative())
        OTHER_NOT_IMPLEMENTED();
    else{
        Matrix<T,m> dGdD[d+1][d+1];
        for(int t=0;t<strain->elements.size();t++){
            for(int i=0;i<d;i++) for(int j=0;j<m;j++){
                Matrix<T,m,d> dDs;
                dDs(j,i) = 1;
                Matrix<T,m,d> dG = U[t]*(Be_scales[t]*dPi_dFe[t].differential(U[t].transpose_times(dDs)*De_inverse_hat[t]).times_transpose(De_inverse_hat[t]));
                for(int k=0;k<d;k++)
                    dGdD[k+1][i+1].set_column(j,dG.column(k));}
            Matrix<T,m> sum;
            for(int i=0;i<d;i++){
                Matrix<T,m> sum_i;
                for(int j=0;j<d;j++)
                    sum_i -= dGdD[i+1][j+1];
                dGdD[i+1][0] = sum_i;
                sum -= sum_i;}
            dGdD[0][0] = sum;
            Vector<int,d+1> nodes = strain->elements[t];
            for(int j=0;j<d+1;j++) for(int i=j;i<d+1;i++)
                matrix.add_entry(nodes[i],nodes[j],dGdD[i][j]);}}
}

template<class TV,int d> typename TV::Scalar FiniteVolume<TV,d>::
damping_energy(RawArray<const TV> V) const
{
    T energy=0;
    for(int t=0;t<strain->elements.size();t++){
        Matrix<T,d> Fe_dot_hat = In_Plane<d>(U[t]).transpose_times(strain->Ds(V,t))*De_inverse_hat[t];
        energy -= Be_scales[t]*model->damping_energy(Fe_hat[t],Fe_dot_hat,t);}
    return energy;
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_damping_force(RawArray<TV> F,RawArray<const TV> V) const
{
    for(int t=0;t<strain->elements.size();t++){
        Matrix<T,m,d> Up = In_Plane<d>(U[t]);
        Matrix<T,d> Fe_dot_hat = Up.transpose_times(strain->Ds(V,t))*De_inverse_hat[t];
        Matrix<T,m,d> forces = Up*model->P_From_Strain_Rate(Fe_hat[t],Fe_dot_hat,Be_scales[t],t).times_transpose(De_inverse_hat[t]);
        strain->distribute_force(F,t,forces);}
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_damping_gradient(SolidMatrix<TV>& matrix) const
{
    Matrix<T,m> dGdD[d+1][d+1];
    for(int t=0;t<strain->elements.size();t++){
        Matrix<T,m,d> Up = In_Plane<d>(U[t]);
        for(int i=0;i<d;i++) for(int j=0;j<m;j++){
            Matrix<T,m,d> Ds_dot;
            Ds_dot(j,i) = 1;
            Matrix<T,d> Fe_dot_hat = Up.transpose_times(Ds_dot)*De_inverse_hat[t];
            Matrix<T,m,d> dG = Up*model->P_From_Strain_Rate(Fe_hat[t],Fe_dot_hat,Be_scales[t],t).times_transpose(De_inverse_hat[t]);
            for(int k=0;k<d;k++)
                dGdD[k+1][i+1].set_column(j,dG.column(k));}
        Matrix<T,m> sum;
        for(int i=0;i<d;i++){
            Matrix<T,m> sum_i;
            for(int j=0;j<d;j++)
                sum_i -= dGdD[i+1][j+1];
            dGdD[i+1][0] = sum_i;
            sum -= sum_i;}
        dGdD[0][0] = sum;
        Vector<int,d+1> nodes = strain->elements[t];
        for(int j=0;j<d+1;j++) for(int i=j;i<d+1;i++)
            matrix.add_entry(nodes[i],nodes[j],dGdD[i][j]);}
}

template<class TV,int d> void FiniteVolume<TV,d>::
add_frequency_squared(RawArray<T> frequency_squared) const
{
    Hashtable<int,T> particle_frequency_squared;
    for(int t=0;t<strain->elements.size();t++){
        T elastic_squared = model->maximum_elastic_stiffness(t)/(sqr(strain->rest_altitude(t))*density);
        const Vector<int,d+1>& nodes = strain->elements(t);
        for(int j=0;j<nodes.m;j++){
            T& data = particle_frequency_squared.get_or_insert(nodes[j]);
            data = max(data,elastic_squared);}}
    for(auto& it : particle_frequency_squared)
        frequency_squared[it.key] += it.data;
}

template<class TV,int d> typename TV::Scalar FiniteVolume<TV,d>::
strain_rate(RawArray<const TV> V) const
{
    T strain_rate=0;
    for(int t=0;t<strain->elements.size();t++)
        strain_rate=max(strain_rate,strain->F(V,t).maxabs());
    return strain_rate;
}

template class FiniteVolume<Vector<T,2>,2>;
template class FiniteVolume<Vector<T,3>,2>;
template class FiniteVolume<Vector<T,3>,3>;
}

void wrap_finite_volume()
{
    using namespace other;

    {typedef FiniteVolume<Vector<T,2>,2> Self;
    Class<Self>("FiniteVolume2d")
        .OTHER_INIT(Ref<StrainMeasure<Vector<T,2>,2> >,T,Ref<ConstitutiveModel<T,2> >,Ptr<PlasticityModel<T,2> >)
        ;}

    {typedef FiniteVolume<Vector<T,3>,2> Self;
    Class<Self>("FiniteVolumeS3d")
        .OTHER_INIT(Ref<StrainMeasure<Vector<T,3>,2> >,T,Ref<ConstitutiveModel<T,2> >,Ptr<PlasticityModel<T,2> >)
        ;}

    {typedef FiniteVolume<Vector<T,3>,3> Self;
    Class<Self>("FiniteVolume3d")
        .OTHER_INIT(Ref<StrainMeasure<Vector<T,3>,3> >,T,Ref<ConstitutiveModel<T,3> >,Ptr<PlasticityModel<T,3> >)
        ;}
}

