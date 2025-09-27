/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <stdio.h>
#include <immintrin.h>
#include <windows.h>
#include "BPE.h"
#include "cosine_similarity.h"
#include "repetition_tester.h"
#include "typedef.h"


inline f32
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
    
    return y;
}

f32
SumOfSimdRegister(__m512 Data)
{
    f32 Value1 = 0;
    f32 Value2 = 0;
    f32 Value3 = 0;
    f32 Value4 = 0;
    f32 Total = 0;

    for (u8 x=0; x<16; x++)
    {
        Value1 += (f32) Data[x];
        Value2 += (f32) Data[x+1];
        Value3 += (f32) Data[x+2];
        Value4 += (f32) Data[x+3];
    }
    
    Total = Value1 + Value2 + Value3 + Value4;
    return Total;
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
        Numerator += SumOfSimdRegister(ZMM0);
    }

    for (u16 x = 0; x < SequenceSize; x+=16)
    {
        ZMM0 = _mm512_loadu_ps((__m512 *) &CurSequence[x]);
        ZMM1 = _mm512_loadu_ps((__m512 *) &AgainstSequence[x]);

        ZMM0 = _mm512_mul_ps(ZMM0, ZMM0);
        ZMM1 = _mm512_mul_ps(ZMM1, ZMM1);

        MagnitudeA += SumOfSimdRegister(ZMM0);
        MagnitudeB += SumOfSimdRegister(ZMM1);

    }

    MagnitudeA = FastInverseSqaureRoot(MagnitudeA);
    MagnitudeB = FastInverseSqaureRoot(MagnitudeB);

    Denominator = MagnitudeB * MagnitudeA;
    FinalValue = Numerator * Denominator;

    return FinalValue;
}


void Swap(int *A, int *B)
{
    *A ^= *B;
    *B ^= *A;
    *A ^= *B;
}

int PartitionForCosineDistance(f32 *CosineDistance, u32 *ID, int low, int high)
{
    f32 Pivot = CosineDistance[high];
    int i = low - 1;

    for (int j = 0; j < high; j++)
    {
        if (CosineDistance[j] < Pivot)
        {
            i++;
            Swap((int *) &CosineDistance[i], (int *) &CosineDistance[j]);
            Swap((int *) &ID[i], (int *) &ID[j]);
        }
    }

    Swap((int *) &CosineDistance[i + 1], (int *) &CosineDistance[high]);
    Swap((int *) &ID[i+1], (int *) &ID[high]);

    return i+1;
}

void QuickSort(f32 * CosineDistance, u32 * ID, int high, int low)
{
    if (low < high)
    {
        int PI = PartitionForCosineDistance(CosineDistance, ID, low, high);
        QuickSort(CosineDistance, ID, PI - 1, low);
        QuickSort(CosineDistance, ID, high, PI + 1);
    }
}

/*void
RadixSortNormal(f32 * Values, u32 * ID, f32 * SortedValues, u32 * SortedID,
        u32 ArraySize, u8 Shifts)
{
    u32 Bits[256] = {};
    u32 CurValue;
    __m512 ZMM0;
    __m512i_u ZMM1;
    
    for (u32 x = 0; x < ArraySize; x++)
    {
        CurValue = (*(u32 *)&Values[x]);
        //& 0x7FFFFFFF;
        CurValue = (CurValue >> Shifts) & 0xFF;
        Bits[CurValue]++;
    }

    for (int x = 1; x < 255; x++)
    {
        CurValue = Bits[x];
        Bits[x] = Bits[x-1];
    }

    for (int x = 2; x < 255; x++)
    {
        Bits[x] += Bits[x-1];
    }

    Bits[0] = 0;

    for (u32 x = 0; x < ArraySize; x++)
    {
        CurValue = (*(u32 *)&Values[x]) & 0x7FFFFFFF;
        CurValue = (CurValue >> Shifts) & 0xFF;
        SortedValues[ArraySize - Bits[CurValue] - 1] = Values[x];
        SortedID[ArraySize - Bits[CurValue] - 1] = ID[x];
        Bits[CurValue]++;
    }

    for (u32 x = 0; x < ArraySize; x+=16)
    {
        ZMM0 = _mm512_loadu_ps((__m512 *) &SortedValues[x]);
        ZMM1 = _mm512_loadu_si512((__m512i_u *) &SortedValues[x]);
        _mm512_storeu_ps(&Values[x], ZMM0);
        _mm512_storeu_si512(&Values[x], ZMM0);
    }
}*/

inline u32
SortKeyToU32(f32 SortKey)
{
    u32 Result = *(u32 *) &SortKey;
    
    if (Result & 0x80000000)
    {
        Result = ~Result;
    }
    else
    {
        Result |= 0x80000000;
    }

    return Result;
}


void
RadixSort(f32 * Values, u32 * ID, f32 * SortedValues, u32 * SortedID,
        u32 ArraySize)
{
    f32 * SourceValues = Values;
    f32 * DestValues = SortedValues;
    u32 * SourceID = ID;
    u32 * DestID = SortedID;

    for (u32 ByteIndex = 0; ByteIndex < 4; ++ByteIndex)
    {
        u32 SortKeyOffsets[256] = {};

        for (u32 i = 0; i < ArraySize; ++i)
        {
            u32 RadixValue = *(u32 *) &Values[i];
            u32 RadixPiece = (RadixValue >> (24 - (ByteIndex * 8))) & 0xFF;
            ++SortKeyOffsets[RadixPiece];
        }

        u32 Total = 0;
        for (u32 SortKeyIndex = 0; SortKeyIndex < ArrayCount(SortKeyOffsets);
                    ++SortKeyIndex)
        {
            u32 Count = SortKeyOffsets[SortKeyIndex];
            SortKeyOffsets[SortKeyIndex] = Total;
            Total += Count;
        }

        for (u32 Index = 0; Index < ArraySize; ++Index)
        {
            u32 RadixValue = *(u32 *) &Values[Index];
            u32 RadixPiece = (RadixValue >> (24 - (ByteIndex * 8))) & 0xFF;
            SortedID[SortKeyOffsets[RadixPiece]] = ID[Index];
            SortedValues[SortKeyOffsets[RadixPiece]++] = Values[Index];
        }

        f32 * TempValues = DestValues;
        DestValues = SourceValues;
        SourceValues = TempValues;
        u32 * TempID = DestID;
        DestID = SourceID;
        SourceID = TempID;
    }
}

void
CosSimWorker(struct bpe_array * CompressedArray, u32 ArraySize,
        f32 * TmpCosineDistance, u32 * TmpCosineID, u32 Index,
        f32 * TmpCosineDistancSorted, u32 * TmpCosineIDSorted)
{
    f32 CosineValues;

    bpe_array CurSequence = CompressedArray[Index];
    TmpCosineDistance[Index] = 0;
    TmpCosineID[Index] = CompressedArray[Index].uID;

    for (u32 x = 0; x < ArraySize; x++)
    {
        if (x == Index)
        {
            continue;
        }

        TmpCosineID[x] = CompressedArray[x].uID;
        TmpCosineDistance[x] = CosineSimilarityCalculator(
                CurSequence.faSequence, CompressedArray[x].faSequence,
                512);
    }

    //printf("%.2f\n", TmpCosineDistance[0]);
    //QuickSort(TmpCosineDistance, TmpCosineID, ArraySize, 0);
    RadixSort(TmpCosineDistance, TmpCosineID, TmpCosineDistancSorted,
            TmpCosineIDSorted, ArraySize);
}

void
CosSimWorkerProfiled(struct bpe_array * CompressedArray, u32 ArraySize,
        f32 * TmpCosineDistance, u32 * TmpCosineID, u32 Index,
        f32 * TmpCosineDistancSorted, u32 * TmpCosineIDSorted)
{
    f32 CosineValues;

    bpe_array CurSequence = CompressedArray[Index];
    TmpCosineDistance[Index] = 0;
    TmpCosineID[Index] = CompressedArray[Index].uID;

    u64 CPUFREQ;
    u64 StartCalc;
    u64 EndCalc;

    for (u32 x = 0; x < ArraySize; x++)
    {
        if (x == Index)
        {
            continue;
        }
        StartCalc = read_cpu_timer();
        TmpCosineID[x] = CompressedArray[x].uID;
        TmpCosineDistance[x] = CosineSimilarityCalculator(
                CurSequence.faSequence, CompressedArray[x].faSequence,
                512);
        EndCalc += read_cpu_timer() - StartCalc;
    }

    u64 StartSort = read_cpu_timer();
    //QuickSort(TmpCosineDistance, TmpCosineID, ArraySize, 0);
    RadixSort(TmpCosineDistance, TmpCosineID, TmpCosineDistancSorted,
            TmpCosineIDSorted, ArraySize);
    u64 EndSort = read_cpu_timer() - StartSort;
    //printf("%.2f\n", TmpCosineDistance[0]);

    u64 Total = EndCalc + EndSort;
    printf("sorting Clocks: %llu\n", EndSort);
    /*printf("CPUFREQ: %llu\n", CPUFREQ);

    printf("Total clocks: %llu\n", Total);
    printf("Calcuation clocks: %llu  %.6fs (%.2f%%)\n", EndCalc,
            ((f64) EndCalc / (f64) CPUFREQ),
            ((f64) EndCalc / (f64) Total) * 100);

    printf("Sorting clocks: %llu  %.6fs (%.2f%%)\n", EndSort,
            ((f64) EndSort / (f64) CPUFREQ),
            ((f64) EndSort / (f64) Total) * 100);*/
}


void
FittingValues(u32 Index, struct recommend_data * CosineValueStructs,
        f32 * CosineDistance, u32 * CosineID, u32 ArraySize)
{
    for (u32 x = 0; x < 100; x++)
    {
        CosineValueStructs[Index].DistanceData[x].fDistance = CosineDistance[
            ArraySize - 1 - x];
        CosineValueStructs[Index].DistanceData[x].uID = CosineID[
            ArraySize - 1 - x];
    }
}


void
CosineSimilarityCalc(struct bpe_array * CompressedArray,
        struct recommend_data * CosineValueStructs, u32 ArraySize)
        /*f32 * TmpCosineDistance, u32 * TmpCosineID,
        f32 * TmpCosineDistanceSorted, u32 * TmpCosineIDSorted)*/
{

    static f32 * TmpCosineDistance = (f32 *) malloc(sizeof(f32) * ArraySize);
    static u32 * TmpCosineID = (u32 *) malloc(sizeof(u32) * ArraySize);
    static f32 * TmpCosineDistanceSorted = (f32 *) malloc(sizeof(f32) * ArraySize);
    static u32 * TmpCosineIDSorted = (u32 *) malloc(sizeof(u32) * ArraySize);

    for (u32 x = 0; x < 1; x++)
    {
        CosineValueStructs[x].uID = CompressedArray[x].uID;
        CosSimWorkerProfiled(CompressedArray, ArraySize,
                TmpCosineDistance, TmpCosineID, x, TmpCosineDistanceSorted,
                TmpCosineIDSorted);

        printf("%.2f\n", TmpCosineDistanceSorted[ArraySize - 1]);

        FittingValues(x, CosineValueStructs, TmpCosineDistanceSorted,
                TmpCosineIDSorted, ArraySize);
    }
}


DWORD WINAPI
CosineSimilarityCalcMultiThread(LPVOID lpParameter)
{
    mt_passer_cos FuncParam = GetDataForCosSim();

    u32 ArraySize = FuncParam.ArraySize;
    bpe_array * CompressedArray = GetBpeArray();
    recommend_data * CosineValueStructs = GetCosineValueStructs();


    f32 * TmpCosineDistance = (f32 *) malloc(sizeof(f32) * ArraySize);
    u32 * TmpCosineID = (u32 *) malloc(sizeof(u32) * ArraySize);
    f32 * TmpCosineDistanceSorted = (f32 *) malloc(sizeof(f32) * ArraySize);
    u32 * TmpCosineIDSorted = (u32 *) malloc(sizeof(u32) * ArraySize);

    for (u32 x = FuncParam.StartIndex;
            x < FuncParam.EndIndex + FuncParam.Extra; x++)
    {
        CosineValueStructs[x].uID = CompressedArray[x].uID;
        CosSimWorker(CompressedArray, ArraySize,
                TmpCosineDistance, TmpCosineID, x, TmpCosineDistanceSorted,
                TmpCosineIDSorted);

        FittingValues(x, CosineValueStructs, TmpCosineDistanceSorted,
                TmpCosineIDSorted, ArraySize);

        if (x % 500 == 0)
        {
            printf("%u\n", x);
        }
    }

    return 0;
}
