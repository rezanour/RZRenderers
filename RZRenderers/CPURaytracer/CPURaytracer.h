//=============================================================================
// CPURaytracer.h - Raytracer implemented using the CPU
// Reza Nourai, 2016
//=============================================================================
#pragma once

class CPURaytracer : public BaseObject<IRZRenderer>
{
public:
  static bool Create(const RZRendererCreateParams* params, IRZRenderer** out_renderer);

  virtual uint32_t AddVertices(uint32_t num_vertices, const RZVector3* positions) override;

  virtual void AddMesh(uint32_t num_indices, const uint32_t* indices) override;

  virtual void RenderScene(const RZVector3& viewer_position, const RZQuaternion& viewer_orientation) override;

private:
  struct triangle
  {
    uint32_t i0, i1, i2;
    RZVector3 e01, e12, e20;
    RZVector3 normal;
    float inv_2x_area;
  };

  struct leaf
  {
    int first_triangle;
    int num_triangles;
  };

  struct node
  {
    RZVector3 min[2], max[2]; // 2 bounding boxes
    int child[2];             // >= 0 is node, else -(i+1) is leaf
  };

private:
  CPURaytracer() {}
  virtual ~CPURaytracer();

  bool Initialize(const RZRendererCreateParams* params);

  void BuildNodes();
  int BuildNode(const RZVector3* centroids, const RZVector3* mins, const RZVector3* maxes, int start, int count, const RZVector3& min, const RZVector3& max);

  bool TestRay(const RZVector3& start, const RZVector3& dir, int node_index, float* out_dist, RZVector3* out_normal);
  static bool TestRayBox(const RZVector3& start, const RZVector3& dir, const RZVector3& min, const RZVector3& max);
  static bool TestRayTriangle(const RZVector3& start, const RZVector3& dir, const RZVector3* positions, const triangle& triangle, float* out_dist, RZVector3* out_normal);

private:
  HWND window_ = nullptr;
  HDC framebuffer_hdc_ = nullptr;
  uint32_t* framebuffer_ = nullptr;
  int width_ = 0;
  int height_ = 0;
  float half_width_ = 0.f;
  float half_height_ = 0.f;
  float dist_to_plane_ = 0.f;
  bool nodes_invalidated_ = true;
  int root_node_ = 0;

  std::vector<RZVector3> positions_;
  std::vector<triangle> triangles_;
  std::vector<uint32_t> triangle_indices_;
  std::vector<leaf> leaves_;
  std::vector<node> nodes_;
};

