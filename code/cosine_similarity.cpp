/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <immintrin.h>
#include "BPE.h"
#include "cosine_similarity.h"


f32
FastInverseSqaureRoot(f32 Value)
{
    long i;
    f32 x2, y;
    f32 ThreeHalfs = 1.5F;

    x2 = Value * 0.5F;
    y = Value;
    i = * (long *) &y;
    i = 0x5f3759df - (i >> 1);
    y = * (f32 *) &i;
    y = y * (ThreeHalfs - (x2 * y * y));
    y = y * (ThreeHalfs - (x2 * y * y));
    
    return y;
}

f32
CosineSimilarityCalculator(f32 * CurSequence, f32 * AgainstSequence,
        u16 SequenceSize)
{
    // Cosine Similarity = (A . B) / (||A|| * ||B||)
    __m512 ZMM0;
    __m512 ZMM1;

    f32 Numerator = 0;
    f32 Denominator = 0;

    f32 MagnitudeA = 0;
    f32 MagnitudeB = 0;

    f32 FinalValue = 0;

    for (u16 x = 0; x < SequenceSize; x+=16)
    {
        ZMM0 = _mm512_loadu_ps((__m512 *) &CurSequence[x]);
        ZMM1 = _mm512_loadu_ps((__m512 *) &AgainstSequence[x]);
        ZMM0 = _mm512_mul_ps(ZMM0, ZMM1);
        Numerator += _mm512_reduce_add_ps(ZMM0);
    }

    for (u16 x = 0; x < SequenceSize; x+=16)
    {
        ZMM0 = _mm512_loadu_ps((__m512 *) &CurSequence[x]);
        ZMM1 = _mm512_loadu_ps((__m512 *) &AgainstSequence[x]);

        ZMM0 = _mm512_mul_ps(ZMM0, ZMM0);
        ZMM1 = _mm512_mul_ps(ZMM1, ZMM1);

        MagnitudeA += _mm512_reduce_add_ps(ZMM0);
        MagnitudeB += _mm512_reduce_add_ps(ZMM0);
    }

    MagnitudeA = FastInverSquareRoot(MagnitudeA);
    MagnitudeB = FastInverSquareRoot(MagnitudeB);
    Denominator = MagnitudeB * MagnitudeA;

    FinalValue = Numerator / Denominator;

    return 0.0;
}

void ShiftRightUsingSimd(struct distance_with_id * DistanceArray,
        u32 ArraySize, u32 ID, u32 CurIndex)
{
    __m512i ZMM0;
    distance_with_id TmpStruct;
    u32 TmpStructIndex;
    u8 Remainder = 0;

    if (ArraySize - CurIndex > 16)
    {
        u32 RemainingSize = ArraySize - CurIndex;
        u8 Remainder = RemainingSize % 16;
        u32 NewArraySize = ArraySize - Remainder - 1;

        for (u32 x = CurIndex; x < NewArraySize; x+=16)
        {
            ZMM0 = _mm512_loadu_si512((__m512i *) &DistanceArray[x]);
            _mm512_storeu_si512((__m512i *) &DistanceArray[x + 1], ZMM0);
        }

        for (u16 x = NewArraySize; x < ArraySize - 1; x++)
        {
            DistanceArray[x] = DistanceArray[x + 1];
        }
    }
    else
    {
        for (u16 x = CurIndex; x < ArraySize - 1; x++)
        {
            DistanceArray[x] = DistanceArray[x + 1];
        }
    }
}

void
InsertValueToDistanceData(struct distance_with_id * DistanceArray,
        f32 CosineSimilarityValue, u32 ID)
{
    struct distance_with_id Tmp;
    for (u8 x = 0; x < 100; x++)
    {
        if (CosineSimilarityValue >= DistanceArray[x].fDistance)
        {
            ShiftRightUsingSimd(DistanceArray, 100, ID, x);
            DistanceArray[x].fDistance = CosineSimilarityValue;
            DistanceArray[x].uID = ID;
            break;
        }
    }
}

void
SendWorkQueueToCalculator(struct bpe_array * CurValue,
        struct bpe_array * WholeArray, u32 BPEArraySize,
        struct distance_data * GlobalDistanceData, u32 CompressedArrayIndex)
{
    f32 CosineSimilarityValue = 0.0;

    for (u32 x = 0; x < BPEArraySize; x++)
    {
        if (WholeArray[x].uID == CurValue -> uID)
        {
            continue;
        }

        CosineSimilarityValue = CosineSimilarityCalculator(
                CurValue -> uaSequence, WholeArray[x].uaSequence, 512);

        InsertValueToDistanceData(
                GlobalDistanceData[CompressedArrayIndex].DistanceArray,
                CosineSimilarityValue, CurValue -> uID);
    }
}

void
AddWorkQueueForRecommendation(struct bpe_array * CompressedArray,
        u32 ArraySize, struct distance_data * GlobalDistanceData,
        u32 CompressedArrayIndex)
{
    for (u32 x=0; x < ArraySize; x++)
    {
        SendWorkQueueToCalculator(&CompressedArray[x], CompressedArray,
                ArraySize, GlobalDistanceData, x);
    }
}
