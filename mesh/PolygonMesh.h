//#####################################################################
// Class PolygonMesh
//#####################################################################
//
// PolygonMesh stores immutable topology for a polygon mesh.  The advantage
// of immutability is that we don't have to worry about acceleration structures
// becoming invalid, and we can check validity once at construction time.
//
//#####################################################################
#pragma once

#include <other/core/array/Array.h>
#include <other/core/mesh/forward.h>
#include <other/core/python/Object.h>
#include <other/core/python/Ptr.h>
#include <other/core/python/Ref.h>
namespace other {

class PolygonMesh : public Object {
public:
  OTHER_DECLARE_TYPE
  typedef Object Base;

  const Array<const int> counts; // number of vertices in each polygon
  const Array<const int> vertices; // indices of each polygon flattened into a single array
private:
  const int node_count, half_edge_count;
  mutable Ptr<SegmentMesh> segment_mesh_;
  mutable Ptr<TriangleMesh> triangle_mesh_;

protected:
  PolygonMesh(Array<const int> counts, Array<const int> vertices);
public:

  int nodes() const {
    return node_count;
  }

  ~PolygonMesh();
  Ref<SegmentMesh> segment_mesh() const OTHER_EXPORT;
  Ref<TriangleMesh> triangle_mesh() const OTHER_EXPORT;
};
}