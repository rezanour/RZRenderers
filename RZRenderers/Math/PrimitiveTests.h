//=============================================================================
// PrimitiveTests.h - Collision tests between various primitives
// Reza Nourai, 2016
//=============================================================================
#pragma once

// Simple boolean test of ray and aabb
bool TestRayBox(const RZVector3& start, const RZVector3& dir, const RZVector3& min, const RZVector3& max);
