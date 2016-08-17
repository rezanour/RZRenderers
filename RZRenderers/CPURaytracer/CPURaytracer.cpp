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
    t.e01 = v1 - v0;
    t.e12 = v2 - v1;
    t.e20 = v0 - v2;
    t.normal = RZVector3::Cross(t.e01, -t.e20);
    t.inv_2x_area = 1.f / t.normal.Length();
    t.normal *= t.inv_2x_area;  // normalize
    triangles_.push_back(t);
  }

  tree_invalidated_ = true;
}

void CPURaytracer::RenderScene(const RZVector3& viewer_position, const RZQuaternion& viewer_orientation)
{
  UNREFERENCED_PARAMETER(viewer_orientation);

  if (tree_invalidated_)
  {
    RebuildTree();
  }

  for (int y = 0; y < height_; ++y)
  {
    for (int x = 0; x < width_; ++x)
    {
      RZVector3 dir{ x - half_width_, half_height_ - y, dist_to_plane_ };
      dir.Normalize();

      uint32_t color = 0xFF000066; // background

      bool hit = false;
      float dist = 0.f, min_dist = FLT_MAX;
      RZVector3 norm{}, min_norm{};

      if (tree_.TraceRay(viewer_position, dir, &scratch_hits_))
      {
        for (int i = 0; i < (int)scratch_hits_.size(); ++i)
        {
          if (TestRayTriangle(viewer_position, dir, positions_.data(), triangles_[scratch_hits_[i]], &dist, &norm))
          {
            if (dist < min_dist)
            {
              min_dist = dist;
              min_norm = norm;
            }
            hit = true;
          }
        }

        if (hit)
        {
          static const RZVector3 light_dir = RZVector3::Normalize(RZVector3{ 1.f, 1.f, -1.f });
          float d = std::min(std::max(0.f, RZVector3::Dot(light_dir, min_norm)), 1.f);
          uint32_t c = (uint32_t)(uint8_t)(255 * d);
          color = 0xFF000000 | (c << 16) | (c << 8) | c;
          //color = 0xFFFFFFFF;
        }
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

void CPURaytracer::RebuildTree()
{
  std::vector<RZVector3> centroids(triangles_.size());
  std::vector<RZVector3> mins(triangles_.size());
  std::vector<RZVector3> maxes(triangles_.size());

  RZVector3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
  RZVector3 max = -min;

  for (uint32_t i = 0; i < triangles_.size(); ++i)
  {
    triangle& t = triangles_[i];
    RZVector3& v0 = positions_[t.i0];
    RZVector3& v1 = positions_[t.i1];
    RZVector3& v2 = positions_[t.i2];
    mins[i] = RZVector3::Min(RZVector3::Min(v0, v1), v2);
    maxes[i] = RZVector3::Max(RZVector3::Max(v0, v1), v2);
    centroids[i] = (v0 + v1 + v2) / 3.f;

    min = RZVector3::Min(min, mins[i]);
    max = RZVector3::Max(max, maxes[i]);
  }

  tree_.Rebuild(centroids.data(), mins.data(), maxes.data(), 0, (int)triangles_.size(), min, max);
  tree_invalidated_ = false;
}

bool CPURaytracer::TestRayTriangle(
  const RZVector3& start, const RZVector3& dir,
  const RZVector3* positions, const triangle& triangle,
  float* out_dist, RZVector3* out_normal)
{
  float cosA = RZVector3::Dot(-triangle.normal, dir);
  if (cosA <= 0)
    return false;

  RZVector3 v0 = positions[triangle.i0];
  RZVector3 v1 = positions[triangle.i1];
  RZVector3 v2 = positions[triangle.i2];

  float d = RZVector3::Dot(start - v0, triangle.normal);
  float h = d / cosA;
  RZVector3 p = start + dir * h;

  // is p inside triangle?
  float r = RZVector3::Dot(RZVector3::Cross(triangle.e01, p - v0), triangle.normal);
  if (r < 0)
    return false;

  float s = RZVector3::Dot(RZVector3::Cross(triangle.e12, p - v1), triangle.normal);
  if (s < 0)
    return false;

  float t = RZVector3::Dot(RZVector3::Cross(triangle.e20, p - v2), triangle.normal);
  if (t < 0)
    return false;

  *out_dist = h;
  *out_normal = triangle.normal;
  return true;
}