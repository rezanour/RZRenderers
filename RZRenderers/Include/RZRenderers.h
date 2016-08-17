//=============================================================================
// RZRenderers - Collection of experimental 3D renderers.
// Reza Nourai, 2016
//=============================================================================
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef __cplusplus
#error Currently, only C++ supported.
#endif

typedef struct
{
  float x, y, z;
} RZVector3;

typedef struct
{
  float x, y, z, w;
} RZQuaternion;

typedef enum
{
  RZRenderer_CPURaytracer = 0,
  RZRenderer_Force32Bits = 0xFFFFFFFF,
} RZRendererType;

typedef struct
{
  void* WindowHandle;
  int32_t RenderWidth;  // Can be different than window's client area
  int32_t RenderHeight; // Can be different than window's client area
  float HorizFOV;       // Horizontal field of view angle, in radians
} RZRendererCreateParams;

struct __declspec(novtable) IRZRenderer
{
  virtual void AddRef() = 0;
  virtual void Release() = 0;

  virtual uint32_t AddVertices(
    uint32_t num_vertices,
    const RZVector3* positions) = 0;

  virtual void AddMesh(
    uint32_t num_indices,
    const uint32_t* indices) = 0;

  virtual void RenderScene(
    const RZVector3& viewer_position,
    const RZQuaternion& viewer_orientation) = 0;
};

bool __stdcall RZRendererCreate(RZRendererType type,
  const RZRendererCreateParams* params,
  IRZRenderer** out_renderer);
