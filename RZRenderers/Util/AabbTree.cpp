//=============================================================================
// AabbTree.cpp - Bounding volume hierarchy based on axis aligned boxes
// Reza Nourai, 2016
//=============================================================================
#include "Precomp.h"
#include "AabbTree.h"
#include "Math/PrimitiveTests.h"

void AabbTree::Rebuild(
  const RZVector3* centroids, const RZVector3* mins, const RZVector3* maxes,
  int start, int count, const RZVector3& min, const RZVector3& max)
{
  indices_.clear();
  leaves_.clear();
  nodes_.clear();

  for (int i = 0; i < count; ++i)
  {
    indices_.push_back(i);
  }

  root_node_ = BuildNode(centroids, mins, maxes, start, count, min, max);
}

bool AabbTree::TraceRay(
  const RZVector3& start, const RZVector3& dir,
  std::vector<uint32_t>* out_primitives) const
{
  if (!out_primitives)
    return false;

  out_primitives->clear();
  return TraceRay(start, dir, root_node_, out_primitives);
}

int AabbTree::BuildNode(
  const RZVector3* centroids, const RZVector3* mins, const RZVector3* maxes,
  int start, int count, const RZVector3& min, const RZVector3& max)
{
  if (count < MaxPrimitivesInLeaf)
  {
    // create a leaf
    leaves_.push_back(leaf{ start, count });
    return -(int)leaves_.size();
  }
  else
  {
    // create a node
    node n{};

    // find longest axis. divide there
    int axis = 0;
    float len = max.x - min.x;
    if (max.y - min.y > len)
    {
      axis = 1;
      len = max.y - min.y;
    }
    if (max.z - min.z > len)
    {
      axis = 2;
      len = max.z - min.z;
    }

    for (int tries = 0; tries < 3; ++tries)
    {
      float value = (&min.x)[axis] + len * 0.5f;

      // sort triangles based on centroids
      int begin = start;
      int end = start + count - 1;

      n.min[0] = n.min[1] = RZVector3{ FLT_MAX, FLT_MAX, FLT_MAX };
      n.max[0] = n.max[1] = -n.min[0];

      while (begin <= end)
      {
        if ((&centroids[indices_[begin]].x)[axis] > value)
        {
          std::swap(indices_[begin], indices_[end]);

          n.min[1] = RZVector3::Min(n.min[1], mins[indices_[end]]);
          n.max[1] = RZVector3::Max(n.max[1], maxes[indices_[end]]);
          --end;
        }
        else
        {
          n.min[0] = RZVector3::Min(n.min[0], mins[indices_[begin]]);
          n.max[0] = RZVector3::Max(n.max[0], maxes[indices_[begin]]);
          ++begin;
        }
      }

      int count0 = begin - start;
      int count1 = count - count0;

      if (count0 > 0 && count1 > 0)
      {
        // success
        n.child[0] = BuildNode(centroids, mins, maxes, start, count0, n.min[0], n.max[0]);
        n.child[1] = BuildNode(centroids, mins, maxes, begin, count1, n.min[1], n.max[1]);
        nodes_.push_back(n);
        return (int)nodes_.size() - 1;
      }
      else
      {
        // not a good split. Try next axis
        if (++axis > 2)
        {
          axis = 0;
        }
        len = (&max.x)[axis] - (&min.x)[axis];
      }
    }

    assert(false);
    return 0;
  }
}

bool AabbTree::TraceRay(
  const RZVector3& start, const RZVector3& dir,
  int node_index, std::vector<uint32_t>* out_primitives) const
{
  const node& n = nodes_[node_index];
  bool hit1 = false, hit2 = false;

  if (TestRayBox(start, dir, n.min[0], n.max[0]))
  {
    if (n.child[0] >= 0)
    {
      hit1 = TraceRay(start, dir, n.child[0], out_primitives);
    }
    else
    {
      const leaf& leaf = leaves_[-(n.child[0] + 1)];
      const uint32_t* indices = indices_.data();
      out_primitives->insert(out_primitives->end(), indices + leaf.start, indices + leaf.start + leaf.count);
      hit1 = true;
    }
  }

  if (TestRayBox(start, dir, n.min[1], n.max[1]))
  {
    if (n.child[1] >= 0)
    {
      hit2 = TraceRay(start, dir, n.child[1], out_primitives);
    }
    else
    {
      const leaf& leaf = leaves_[-(n.child[1] + 1)];
      const uint32_t* indices = indices_.data();
      out_primitives->insert(out_primitives->end(), indices + leaf.start, indices + leaf.start + leaf.count);
      hit2 = true;
    }
  }

  return hit1 || hit2;
}
