#pragma once

#include <gba_types.h>

#define FX_BITS 8
#define FX_UNIT (1 << FX_BITS)
#define FX_MASK (FX_UNIT - 1)

#define INT2FX(a) ((a) << FX_BITS)
#define FX2INT(a) ((a) >> FX_BITS)

#define FLOAT2FX(a) ((int)((a) * FX_UNIT))
#define FX2FLOAT(a) ((float)(a) / FX_UNIT)

#define FX_MUL(a,b) (((a)*(b)) >> FX_BITS)
#define FX_DIV(a,b) (((a) << FX_BITS) / (b))