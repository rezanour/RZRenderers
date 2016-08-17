//=============================================================================
// CPURaytracer.cpp - Raytracer implemented using the CPU
// Reza Nourai, 2016
//=============================================================================
#include "Precomp.h"
#include "CPURaytracer.h"

CPURaytracer::~CPURaytracer()
{
  framebuffer_ = nullptr;

  if (framebuffer_hdc_)
  {
    DeleteDC(framebuffer_hdc_);
    framebuffer_hdc_ = nullptr;
  }
}

bool CPURaytracer::Create(const RZRendererCreateParams* params, IRZRenderer** out_renderer)
{
  CPURaytracer* renderer = new CPURaytracer();
  if (!renderer->Initialize(params))
  {
    delete renderer;
    renderer = nullptr;
    return false;
  }

  *out_renderer = renderer;
  return true;
}

bool CPURaytracer::Initialize(const RZRendererCreateParams* params)
{
  window_ = (HWND)params->WindowHandle;

  HDC hdc = GetDC(window_);
  if (!hdc)
  {
    assert(false);
    return false;
  }

  framebuffer_hdc_ = CreateCompatibleDC(hdc);

  // done with the hwnd's hdc.
  ReleaseDC(window_, hdc);

  if (!framebuffer_hdc_)
  {
    assert(false);
    return false;
  }

  width_ = params->RenderWidth;
  height_ = params->RenderHeight;
  half_width_ = width_ * 0.5f;
  half_height_ = height_ * 0.5f;
  dist_to_plane_ = half_width_ / tanf(params->HorizFOV * 0.5f);

  BITMAPINFO bmi{};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width_;
  bmi.bmiHeader.biHeight = -height_; // Negative means top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  HBITMAP bitmap = CreateDIBSection(framebuffer_hdc_, &bmi, DIB_RGB_COLORS, (PVOID*)&framebuffer_, nullptr, 0);
  if (!bitmap)
  {
    assert(false);
    return false;
  }

  // Select the bitmap (this takes a reference on it)
  SelectObject(framebuffer_hdc_, bitmap);

  // Delete the object (the DC still has a reference)
  DeleteObject(bitmap);

  return true;
}

uint32_t CPURaytracer::AddVertices(uint32_t num_vertices, const RZVector3* positions)
{
  uint32_t index = (uint32_t)positions_.size();
  positions_.insert(positions_.end(), positions, positions + num_vertices);
  return index;
}

void CPURaytracer::AddMesh(uint32_t num_indices, const uint32_t* indices)
{
  for (uint32_t i = 0; i < num_indices; i += 3)
  {
    triangle t;
    t.i0 = indices[i];
    t.i1 = indices[i + 1];
    t.i2 = indices[i + 2];
    RZVector3 v0 = positions_[t.i0];
    RZVector3 v1 = positions_[t.i1];
    RZVector3 v2 = positions_[t.i2];
    t.e01 = RZVector3_sub(v1, v0);
    t.e12 = RZVector3_sub(v2, v1);
    t.e20 = RZVector3_sub(v0, v2);
    RZVector3 norm = RZVector3_cross(t.e01, RZVector3_neg(t.e20));
    t.inv_2x_area = 1.f / RZVector3_length(norm);
    t.normal = RZVector3_normalize(norm);
    triangles_.push_back(t);
  }

  nodes_invalidated_ = true;
}

void CPURaytracer::RenderScene(const RZVector3& viewer_position, const RZQuaternion& viewer_orientation)
{
  UNREFERENCED_PARAMETER(viewer_orientation);

  if (nodes_invalidated_)
  {
    BuildNodes();
  }

  for (int y = 0; y < height_; ++y)
  {
    for (int x = 0; x < width_; ++x)
    {
      RZVector3 dir{ x - half_width_, half_height_ - y, dist_to_plane_ };
      dir = RZVector3_normalize(dir);

      uint32_t color = 0xFF000066; // background

      float dist = 0.f;
      RZVector3 norm{};
      if (TestRay(viewer_position, dir, root_node_, &dist, &norm))
      {
        static const RZVector3 light_dir = RZVector3_normalize(RZVector3{ 1.f, 1.f, -1.f });
        float d = std::min(std::max(0.f, RZVector3_dot(light_dir, norm)), 1.f);
        uint32_t c = (uint32_t)(uint8_t)(255 * d);
        color = 0xFF000000 | (c << 16) | (c << 8) | c;
        //color = 0xFFFFFFFF;
      }

      framebuffer_[y * width_ + x] = color;
    }
  }

  RECT client_rect{};
  GetClientRect(window_, &client_rect);

  HDC hdc = GetDC(window_);

  int client_width = client_rect.right - client_rect.left;
  int client_height = client_rect.bottom - client_rect.top;

  if (client_width != width_ || client_height != height_)
  {
    StretchBlt(hdc, 0, 0, client_width, client_height, framebuffer_hdc_, 0, 0, width_, height_, SRCCOPY);
  }
  else
  {
    BitBlt(hdc, 0, 0, width_, height_, framebuffer_hdc_, 0, 0, SRCCOPY);
  }

  ReleaseDC(window_, hdc);
}

void CPURaytracer::BuildNodes()
{
  triangle_indices_.clear();
  leaves_.clear();
  nodes_.clear();
  std::vector<RZVector3> centroids(triangles_.size());
  std::vector<RZVector3> mins(triangles_.size());
  std::vector<RZVector3> maxes(triangles_.size());

  RZVector3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
  RZVector3 max = RZVector3_neg(min);

  for (uint32_t i = 0; i < triangles_.size(); ++i)
  {
    triangle_indices_.push_back(i);
    triangle& t = triangles_[i];
    RZVector3& v0 = positions_[t.i0];
    RZVector3& v1 = positions_[t.i1];
    RZVector3& v2 = positions_[t.i2];
    mins[i] = RZVector3_min(RZVector3_min(v0, v1), v2);
    maxes[i] = RZVector3_max(RZVector3_max(v0, v1), v2);
    centroids[i] = RZVector3_mul(RZVector3_add(RZVector3_add(v0, v1), v2), 1.f / 3.f);

    min = RZVector3_min(min, mins[i]);
    max = RZVector3_max(max, maxes[i]);
  }

  root_node_ = BuildNode(centroids.data(), mins.data(), maxes.data(), 0, (uint32_t)triangles_.size(), min, max);
  nodes_invalidated_ = false;
}

int CPURaytracer::BuildNode(const RZVector3* centroids, const RZVector3* mins, const RZVector3* maxes, int start, int count, const RZVector3& min, const RZVector3& max)
{
  if (count < 8)
  {
    // create a leaf
    leaf l;
    l.first_triangle = start;
    l.num_triangles = count;
    leaves_.push_back(l);
    return -(int)leaves_.size();
  }
  else
  {
    // create a node
    node n;

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
      n.max[0] = n.max[1] = RZVector3_neg(n.min[0]);

      while (begin <= end)
      {
        if ((&centroids[triangle_indices_[begin]].x)[axis] > value)
        {
          std::swap(triangle_indices_[begin], triangle_indices_[end]);

          n.min[1] = RZVector3_min(n.min[1], mins[triangle_indices_[end]]);
          n.max[1] = RZVector3_max(n.max[1], maxes[triangle_indices_[end]]);
          --end;
        }
        else
        {
          n.min[0] = RZVector3_min(n.min[0], mins[triangle_indices_[begin]]);
          n.max[0] = RZVector3_max(n.max[0], maxes[triangle_indices_[begin]]);
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

bool CPURaytracer::TestRay(const RZVector3& start, const RZVector3& dir, int node_index, float* out_dist, RZVector3* out_normal)
{
  node& n = nodes_[node_index];
  bool hit1 = false, hit2 = false;
  float dist = FLT_MAX;
  float min_dist = FLT_MAX;
  RZVector3 norm{};

  if (TestRayBox(start, dir, n.min[0], n.max[0]))
  {
    if (n.child[0] >= 0)
    {
      hit1 = TestRay(start, dir, n.child[0], &min_dist, out_normal);
    }
    else
    {
      leaf& leaf = leaves_[-(n.child[0] + 1)];
      for (int i = 0; i < leaf.num_triangles; ++i)
      {
        if (TestRayTriangle(start, dir, positions_.data(), triangles_[triangle_indices_[leaf.first_triangle + i]], &dist, &norm))
        {
          if (dist < min_dist)
          {
            min_dist = dist;
            *out_normal = norm;
          }
          hit1 = true;
        }
      }
    }
  }

  if (TestRayBox(start, dir, n.min[1], n.max[1]))
  {
    if (n.child[1] >= 0)
    {
      hit2 = TestRay(start, dir, n.child[1], &dist, &norm);
      if (dist < min_dist)
      {
        min_dist = dist;
        *out_normal = norm;
      }
    }
    else
    {
      leaf& leaf = leaves_[-(n.child[1] + 1)];
      for (int i = 0; i < leaf.num_triangles; ++i)
      {
        if (TestRayTriangle(start, dir, positions_.data(), triangles_[triangle_indices_[leaf.first_triangle + i]], &dist, &norm))
        {
          if (dist < min_dist)
          {
            min_dist = dist;
            *out_normal = norm;
          }
          hit2 = true;
        }
      }
    }
  }

  *out_dist = min_dist;
  return hit1 || hit2;
}

bool CPURaytracer::TestRayBox(const RZVector3& start, const RZVector3& dir, const RZVector3& min, const RZVector3& max)
{
  float s, px, py, pz;

  // if the start is already inside the box, then it's a hit
  if (
    start.x >= min.x && start.x <= max.x &&
    start.y >= min.y && start.y <= max.y &&
    start.z >= min.z && start.z <= max.z)
  {
    return true;
  }

  // if dir is moving towards the min of the box, and
  // we're outside of the min, then that's a potential hit
  if (dir.x > 0 && start.x < min.x)
  {
    s = (min.x - start.x) / dir.x;
    py = start.y + dir.y * s;
    pz = start.z + dir.z * s;
    if (
      py >= min.y && py <= max.y &&
      pz >= min.z && pz <= max.z)
    {
      return true;
    }
  }
  else if (dir.x < 0 && start.x > max.x)
  {
    s = (max.x - start.x) / dir.x;
    py = start.y + dir.y * s;
    pz = start.z + dir.z * s;
    if (
      py >= min.y && py <= max.y &&
      pz >= min.z && pz <= max.z)
    {
      return true;
    }
  }

  if (dir.y > 0 && start.y < min.y)
  {
    s = (min.y - start.y) / dir.y;
    px = start.x + dir.x * s;
    pz = start.z + dir.z * s;
    if (
      px >= min.x && px <= max.x &&
      pz >= min.z && pz <= max.z)
    {
      return true;
    }
  }
  else if (dir.y < 0 && start.y > max.y)
  {
    s = (max.y - start.y) / dir.y;
    px = start.x + dir.x * s;
    pz = start.z + dir.z * s;
    if (
      px >= min.x && px <= max.x &&
      pz >= min.z && pz <= max.z)
    {
      return true;
    }
  }

  if (dir.z > 0 && start.z < min.z)
  {
    s = (min.z - start.z) / dir.z;
    px = start.x + dir.x * s;
    py = start.y + dir.y * s;
    if (
      px >= min.x && px <= max.x &&
      py >= min.y && py <= max.y)
    {
      return true;
    }
  }
  else if (dir.z < 0 && start.z > max.z)
  {
    s = (max.z - start.z) / dir.z;
    px = start.x + dir.x * s;
    py = start.y + dir.y * s;
    if (
      px >= min.x && px <= max.x &&
      py >= min.y && py <= max.y)
    {
      return true;
    }
  }

  return false;
}

bool CPURaytracer::TestRayTriangle(const RZVector3& start, const RZVector3& dir, const RZVector3* positions, const triangle& triangle, float* out_dist, RZVector3* out_normal)
{
  float cosA = RZVector3_dot(RZVector3_neg(triangle.normal), dir);
  if (cosA <= 0)
    return false;

  RZVector3 v0 = positions[triangle.i0];
  RZVector3 v1 = positions[triangle.i1];
  RZVector3 v2 = positions[triangle.i2];

  float d = RZVector3_dot(RZVector3_sub(start, v0), triangle.normal);
  float h = d / cosA;
  RZVector3 p = RZVector3_add(start, RZVector3_mul(dir, h));

  // is p inside triangle?
  float r = RZVector3_dot(RZVector3_cross(triangle.e01, RZVector3_sub(p, v0)), triangle.normal);
  if (r < 0)
    return false;

  float s = RZVector3_dot(RZVector3_cross(triangle.e12, RZVector3_sub(p, v1)), triangle.normal);
  if (s < 0)
    return false;

  float t = RZVector3_dot(RZVector3_cross(triangle.e20, RZVector3_sub(p, v2)), triangle.normal);
  if (t < 0)
    return false;

  *out_dist = h;
  *out_normal = triangle.normal;
  return true;
}