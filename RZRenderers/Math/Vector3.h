//=============================================================================
// Vector3.h - Helper functions for using RZVector3
// Reza Nourai, 2016
//=============================================================================
#pragma once

inline RZVector3 RZVector3_add(const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

inline RZVector3 RZVector3_sub(const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

inline RZVector3 RZVector3_neg(const RZVector3& v)
{
  return RZVector3{ -v.x, -v.y, -v.z };
}

inline RZVector3 RZVector3_mul(const RZVector3& v, float s)
{
  return RZVector3{ v.x * s, v.y * s, v.z * s };
}

inline float RZVector3_dot(const RZVector3& v1, const RZVector3& v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline RZVector3 RZVector3_cross(const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3
  {
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x,
  };
}

inline float RZVector3_length(const RZVector3& v)
{
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline RZVector3 RZVector3_normalize(const RZVector3& v)
{
  float inv_len = 1.f / RZVector3_length(v);
  return RZVector3{ v.x * inv_len, v.y * inv_len, v.z * inv_len };
}

inline RZVector3 RZVector3_min(const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3{ std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z) };
}

inline RZVector3 RZVector3_max(const RZVector3& v1, const RZVector3& v2)
{
  return RZVector3{ std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z) };
}
