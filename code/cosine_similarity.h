/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#ifndef W__CODE_CS_H_
#define W__CODE_CS_H_

#include <stdint.h>
#include <windows.h>
#include "BPE.h"
#include "typedef.h"


struct distance_with_id{
    f32 fDistance;
    u32 uID;
};

struct distance_data{
    struct distance_with_id * DistanceArray;
    u32 uID;
};

#endif // W__CODE_CS_H_
