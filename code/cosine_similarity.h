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

//volatile u64 CosSimProfiler;
//volatile u64 SorterProfiler;

static u32 GlobalWorkQueueIndex;

struct cosine_distance {
    f32 fDistance;
    u32 uID;
};

struct recommend_data{
    cosine_distance DistanceData[100];
    u32 uID;
};

struct mt_passer_cos {
    u32 ArraySize;
    struct bpe_array * CompressedArray;
    struct recommend_data * CosineValueStructs;
    u32 StartIndex;
    u32 EndIndex;
    u8 Extra = 0;
};

void
CosineSimilarityCalc(struct bpe_array * CompressedArray,
        struct recommend_data * CosineValueStructs, u32 ArraySize);
/*        f32 * TmpCosineDistance, u32 * TmpCosineID,
        f32 * TmpCosineDistanceSorted, u32 * TmpCosineIDSorted);*/

struct mt_passer_cos
GetDataForCosSim();

struct bpe_array *
GetBpeArray();

struct recommend_data *
GetCosineValueStructs();

DWORD WINAPI
CosineSimilarityCalcMultiThread(LPVOID lpParameter);

#endif // W__CODE_CS_H_
