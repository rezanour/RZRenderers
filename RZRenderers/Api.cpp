//=============================================================================
// Api.cpp - Public API implementation
// Reza Nourai, 2016
//=============================================================================
#include "Precomp.h"
#include "CPURaytracer/CPURaytracer.h"

bool __stdcall RZRendererCreate(RZRendererType type,
  const RZRendererCreateParams* params,
  IRZRenderer** out_renderer)
{
  if (!params || !out_renderer)
  {
    assert(false);
    return false;
  }

  *out_renderer = nullptr;

  switch (type)
  {
  case RZRenderer_CPURaytracer:
    return CPURaytracer::Create(params, out_renderer);

  default:
    assert(false);
    return false;
  }
}
