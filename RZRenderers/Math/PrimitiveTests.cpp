//=============================================================================
// PrimitiveTests.cpp - Collision tests between various primitives
// Reza Nourai, 2016
//=============================================================================
#include "Precomp.h"
#include "PrimitiveTests.h"

bool TestRayBox(const RZVector3& start, const RZVector3& dir, const RZVector3& min, const RZVector3& max)
{
  // This version with fewer branches is just a tiny bit faster than the traditional one.
  // And will be easier to port to SIMD later
  float rx = start.x, ry = start.y, rz = start.z;

  float s;

  if (dir.x > 0 && rx < min.x)
  {
    s = (min.x - rx) / dir.x;
    rx = min.x;
    ry = ry + dir.y * s;
    rz = rz + dir.z * s;
  }
  else if (dir.x < 0 && rx > max.x)
  {
    s = (max.x - rx) / dir.x;
    rx = max.x;
    ry = ry + dir.y * s;
    rz = rz + dir.z * s;
  }

  if (dir.y > 0 && ry < min.y)
  {
    s = (min.y - ry) / dir.y;
    ry = min.y;
    rx = rx + dir.x * s;
    rz = rz + dir.z * s;
  }
  else if (dir.y < 0 && ry > max.y)
  {
    s = (max.y - ry) / dir.y;
    ry = max.y;
    rx = rx + dir.x * s;
    rz = rz + dir.z * s;
  }

  if (dir.z > 0 && rz < min.z)
  {
    s = (min.z - rz) / dir.z;
    rz = min.z;
    rx = rx + dir.x * s;
    ry = ry + dir.y * s;
  }
  else if (dir.z < 0 && rz > max.z)
  {
    s = (max.z - rz) / dir.z;
    rz = max.z;
    rx = rx + dir.x * s;
    ry = ry + dir.y * s;
  }

  return (rx >= min.x && rx <= max.x &&
    ry >= min.y && ry <= max.y &&
    rz >= min.z && rz <= max.z);
}
