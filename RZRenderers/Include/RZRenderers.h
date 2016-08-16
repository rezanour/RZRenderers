#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#define PUBLIC_FUNCTION extern "C"
#else
#define PUBLIC_FUNCTION
#endif

typedef enum
{
  RZRenderer_CPURaytracer = 0,
  RZRenderer_Force32Bits = 0xFFFFFFFF,
} RZRendererType;

// 0 is an invalid handle value
typedef uint32_t RZRendererHandle;

PUBLIC_FUNCTION RZRendererHandle RZRendererCreate(RZRendererType type);
PUBLIC_FUNCTION void RZRendererDestroy(RZRendererHandle renderer);
