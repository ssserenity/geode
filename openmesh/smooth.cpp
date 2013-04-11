#ifdef USE_OPENMESH
#include <other/core/openmesh/smooth.h>
#include <gmm/gmm.h>
#include <other/core/python/wrap.h>
#include <other/core/utility/stream.h>

namespace other {

typedef gmm::row_matrix<gmm::wsvector<real>> Mx;
typedef Vector<real,3> TV;

using std::make_pair;

Ref<TriMesh> smooth_mesh(TriMesh &m, real t, real lambda, bool bilaplace, int val) {
  Ref<TriMesh> M = m.copy();
  M->garbage_collection();


  /*
  vector<HalfedgeHandle> to_split;

  for(auto f : M->face_handles()){
      auto p0 = M->point(v);
      auto p1 = M->point(M->to_vertex_handle(ohi.handle()));
      auto p2 = M->point(M->from_vertex_handle(M->prev_halfedge_handle(ohi.handle())));
      auto d0 = p1-p0;
      auto d1 = p2-p1;
      auto d2 = p0-p2;

      real m0 = d0.sqr_magnitude();
      real m2 = d2.sqr_magnitude();

      d0.normalize(); d1.normalize(); d2.normalize();

      real phi20 = acos( max(-1.,min(1.,dot(d0,-d2))) );
      real phi01 = acos( max(-1.,min(1.,dot(-d0,d1))) );
      real phi12 = acos(max(-1.,min(1.,dot(d2,-d1))));
  }
  */


  auto bb = M->bounding_box();
  real zz = bb.sizes().z;

  for (auto v : M->vertex_handles()){
    if(abs(M->point(v).z-bb.center().z) > .3*zz) M->status(v).set_locked(true);
  }

  for (auto v : m.vertex_handles()){
    if(abs(m.point(v).z-bb.center().z) > .3*zz) m.status(v).set_locked(true);
  }

  Array<VertexHandle> VI;
  Array<VertexHandle> VB;

  unordered_map<int,VertexHandle,Hasher> id_to_handle;
  unordered_map<VertexHandle,int,Hasher> handle_to_id;

  for(auto v : M->vertex_handles()){
    if(M->status(v).locked()) VB.append(v);
    else VI.append(v);
  }

//  OTHER_ASSERT((int)VI.size() == (int)M->n_vertices());
  Array<VertexHandle> V(VI);
  V.append_elements(VB);

  for(int i=0; i < (int)V.size(); ++i){
    id_to_handle.insert(make_pair(i,V[i]));
    handle_to_id.insert(make_pair(V[i],i));
  }

  int n = VI.size();
  int e = V.size();

  std::vector<TV> output(n);

  Mx L(e,e);

  real min_a = numeric_limits<real>::infinity();

  for(auto v : M->vertex_handles()){
    auto neighbors = M->vertex_one_ring(v);
    OTHER_ASSERT(neighbors.size() && v.is_valid());
    int i = handle_to_id[v];
    real W=0.;
    real A=0.;

    for(auto ohi = M->cvoh_iter(v);ohi;++ohi){
      auto fh = M->face_handle(ohi.handle());

      auto p0 = M->point(v);
      auto p1 = M->point(M->to_vertex_handle(ohi.handle()));
      auto p2 = M->point(M->from_vertex_handle(M->prev_halfedge_handle(ohi.handle())));
      auto d0 = p1-p0;
      auto d1 = p2-p1;
      auto d2 = p0-p2;

      real m0 = d0.sqr_magnitude();
      real m2 = d2.sqr_magnitude();

      d0.normalize(); d1.normalize(); d2.normalize();

      real phi20 = acos( max(-1.,min(1.,dot(d0,-d2))) );
      real phi01 = acos( max(-1.,min(1.,dot(-d0,d1))) );
      real phi12 = acos(max(-1.,min(1.,dot(d2,-d1))));

      real obtuse = pi*.5;
      if(dot(d0,-d2) >=0 && dot(-d0,d1) >= 0 && dot(-d1,d2) >=0){
        A+=(1/8.) * (m0/tan(phi12) + m2/tan(phi01));
      }else{
        real a = M->area(fh);
        if(phi20 >= obtuse)
          A+=a/2;
        else A+=a/4;
      }

      /*
      auto t = M->triangle(fh);
      auto c = t.point_from_barycentric_coordinates(TV::ones()*(1/3.));
      A+=cross(d0*.5,(c-p0)).magnitude();
      */
    }

    if(A < min_a) min_a = A;

    /*
    for(auto n : neighbors)
      A+=M->area(M->face_handle(M->halfedge_handle(v,n)));
    */

    for(auto n : neighbors){
      OTHER_ASSERT(n.is_valid() && !M->status(n).deleted());
      //real w = M->cotan_weight(M->edge_handle(M->halfedge_handle(v,n)))/(2.*A);
      real w = 1.;
      L(i,handle_to_id[n]) = -w;
      W+=w;
    }
    L(i,i) = W;
  }

  //cout << min_a << endl;

  Mx U(n,n);
  if(bilaplace) gmm::mult(L,L,U);
  else gmm::mult(gmm::identity_matrix(),L,U);

  /*
  auto AA = gmm::sub_matrix(U, gmm::sub_interval(0, n));
  auto BB = gmm::sub_matrix(U, gmm::sub_interval(0, n), gmm::sub_interval(n, e-n));

  Mx A(n,n);
  gmm::copy(AA,A);

  Mx B(n,e-n);
  gmm::copy(BB,B);

  gmm::scale(B,lambda*t);
  gmm::scale(A,lambda*t);

  Mx In(n,e-n);
  for(int i=0;i<n;++i)
    In(i,i) = 1;

  gmm::add(gmm::identity_matrix(),A);
  gmm::add(In,B);

  gmm::identity_matrix PS;
  gmm::identity_matrix PR;

  //  for(int pass = 0; pass < 5; ++pass){

    std::vector<TV> prev(n);
    for(auto v : VI){
      prev[handle_to_id[v]] = M->point(v);
    }

    for(int i =0; i < 3; ++i){

      std::vector<double> X(n);
      gmm::iteration iter(10E-9);

      vector<double> previ(n);
      for(int k=0;k<(int)prev.size();++k)
        previ[k] = prev[k][i];


        int id_offset = n;
      vector<double> y(e);
      vector<double> RHS(n);

      for(auto v : VB)
        y[handle_to_id[v]-id_offset] = M->point(v)[i];

      gmm::mult(B,y,RHS);

      gmm::scale(RHS,-1);
      gmm::add(previ,RHS);

      gmm::cg(A,X,RHS,PS,PR,iter);
      //gmm::cg(A,X,previ,PS,PR,iter);

      for(int j=0;j<(int)X.size();++j)
        output[j][i] = X[j];

    }

    for(int i=0;i<(int)output.size();++i){
      auto vh = id_to_handle[i];
      OTHER_ASSERT(vh.is_valid() && !M->status(vh).locked());
      M->point(vh) = output[i];
    }
*/

  auto A = gmm::sub_matrix(U, gmm::sub_interval(0, n));
  auto B = gmm::sub_matrix(U, gmm::sub_interval(0, n), gmm::sub_interval(n, e-n));

  //  for(int i =0; i <n; ++i)
  //    A(i,i) += 1e-3;

  std::vector<real> eig(n);
  gmm::dense_matrix<real> evs(n,n);
  gmm::symmetric_qr_algorithm(A,eig,evs);

  real mx = -numeric_limits<real>::infinity();
  real mn = -mx;
  int nev = val;
  for(int i =0; i<n; ++i){
    mx = max(abs(evs(i,nev)),mx);
    mn = min(abs(evs(i,nev)),mn);
  }

  std::cout << "min/max: " << mn << " " << mx << std::endl;

  m.request_vertex_colors();
  for(auto v : m.vertex_handles()){
    if(!m.status(v).locked()){
      real e = abs(evs(handle_to_id[v],nev));
      real c = (e-mn)/(mx-mn);
      m.set_color(v,to_byte_color(TV(c,0,1-c)));
    }
    else m.set_color(v,to_byte_color(TV(.5, .5, .5)));
  }

  gmm::identity_matrix PS;
  gmm::identity_matrix PR;

  for(int i =0; i < 3; ++i){

    std::vector<double> X(n);
    gmm::iteration iter(10E-12);

    int id_offset = n;
    vector<double> y(e-n);
    vector<double> RHS(n);

    for(auto v : VB)
      y[handle_to_id[v]-id_offset] = M->point(v)[i];

    gmm::mult(B,y,RHS);
    gmm::scale(RHS,-1.);

    gmm::cg(A,X,RHS,PS,PR,iter);
    //gmm::cg(A,X,previ,PS,PR,iter);

    for(int j=0;j<(int)X.size();++j)
      output[j][i] = X[j];

  }

  for(int i=0;i<(int)output.size();++i){
    auto vh = id_to_handle[i];
    OTHER_ASSERT(vh.is_valid() && !M->status(vh).locked());
    M->point(vh) = output[i];
  }


  return M;
}

}

using namespace other;

void wrap_smooth() {
  OTHER_FUNCTION(smooth_mesh)
}

#endif
