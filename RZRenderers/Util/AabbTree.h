//=============================================================================
// AabbTree.h - Bounding volume hierarchy based on axis aligned boxes
// Reza Nourai, 2016
//=============================================================================
#pragma once

class AabbTree
{
public:
  AabbTree() {}
  ~AabbTree() {}

  // Clear out and rebuild the Aaabb tree
  void Rebuild(
    const RZVector3* centroids, const RZVector3* mins, const RZVector3* maxes,
    int start, int count, const RZVector3& min, const RZVector3& max);

  // Trace a ray through the tree, returning the list of primitives that could possibly be hit
  bool TraceRay(
    const RZVector3& start, const RZVector3& dir,
    std::vector<uint32_t>* out_primitives) const;

private:
  struct leaf
  {
    int start;
    int count;
  };

  struct node
  {
    RZVector3 min[2], max[2]; // 2 bounding boxes
    int child[2];             // >= 0 is node, else -(i+1) is leaf
  };

private:
  AabbTree(const AabbTree&) = delete;
  AabbTree& operator= (const AabbTree&) = delete;

  int BuildNode(
    const RZVector3* centroids, const RZVector3* mins, const RZVector3* maxes,
    int start, int count, const RZVector3& min, const RZVector3& max);

  bool TraceRay(const RZVector3& start, const RZVector3& dir,
    int node_index, std::vector<uint32_t>* out_primitives) const;

private:
  static const int MaxPrimitivesInLeaf = 32;

  int root_node_ = 0;
  std::vector<node> nodes_;
  std::vector<leaf> leaves_;
  std::vector<uint32_t> indices_;
};
