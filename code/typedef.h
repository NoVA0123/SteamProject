/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#ifndef W__CODE_TYPEDEF_H_
#define W__CODE_TYPEDEF_H_

#include <stdint.h>

typedef uint8_t u8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;

typedef int64_t s64;
typedef int16_t s16;

typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#endif  // W__CODE_TYPEDEF_H_
