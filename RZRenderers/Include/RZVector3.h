//=============================================================================
// RZVector3.h - Vector in 3 dimensions
// Reza Nourai, 2016
//=============================================================================
#pragma once

struct RZVector3
{
  float x, y, z;

  // Member operators
  RZVector3& operator += (const RZVector3& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  RZVector3& operator -= (const RZVector3& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  RZVector3& operator *= (float s)
  {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  RZVector3& operator /= (float s)
  {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }

  // Member helpers
  float Length() const
  {
    return sqrtf(x * x + y * y + z * z);
  }

  void Normalize()
  {
    float inv_len = 1.f / Length();
    x *= inv_len;
    y *= inv_len;
    z *= inv_len;
  }

  // Static helpers
  static float Dot(const RZVector3& v1, const RZVector3& v2)
  {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  static RZVector3 Cross(const RZVector3& v1, const RZVector3& v2)
  {
    return RZVector3
    {
      v1.y * v2.z - v1.z * v2.y,
      v1.z * v2.x - v1.x * v2.z,
      v1.x * v2.y - v1.y * v2.x,
    };
  }

  static RZVector3 Normalize(const RZVector3& v)
  {
    float inv_len = 1.f / v.Length();
    return RZVector3{ v.x * inv_len, v.y * inv_len, v.z * inv_len };
  }

  static RZVector3 Min(const RZVector3& v1, const RZVector3& v2)
  {
    return RZVector3{ std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z) };
  }

  static RZVector3 Max(const RZVector3& v1, const RZVector3& v2)
  {
    return RZVector3{ std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z) };
  }
};

// Global operators
inline RZVector3 operator+ (const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

inline RZVector3 operator- (const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

inline RZVector3 operator- (const RZVector3& v)
{
  return RZVector3{ -v.x, -v.y, -v.z };
}

inline RZVector3 operator* (const RZVector3& v, float s)
{
  return RZVector3{ v.x * s, v.y * s, v.z * s };
}

inline RZVector3 operator/ (const RZVector3& v, float s)
{
  return RZVector3{ v.x / s, v.y / s, v.z / s };
}

inline RZVector3 operator* (float s, const RZVector3& v)
{
  return RZVector3{ v.x * s, v.y * s, v.z * s };
}
