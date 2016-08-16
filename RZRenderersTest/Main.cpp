#include "Precomp.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  RZRendererHandle renderer = RZRendererCreate(RZRenderer_CPURaytracer);
  if (renderer)
  {
    RZRendererDestroy(renderer);
    renderer = 0;
  }

  return 0;
}
