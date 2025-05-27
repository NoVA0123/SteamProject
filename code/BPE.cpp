/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <immintrin.h>
#include <stdio.h>
#include <windows.h>
#include "combiner.h"
#include "repetition_tester.h"
#include "BPE.h"

static u64 LeftShift = 0;
void
CopyTheStringUsingSimd(char * CombinedBuffer, u16 * Sequence, u16 ArraySize)
{
    __m256i YMM0;
    __m512i ZMM1;

    for (u16 x = 0; x < ArraySize; x+=32)
    {
        YMM0 = _mm256_loadu_si256((__m256i *) &CombinedBuffer[x]);
        ZMM1 = _mm512_cvtepi8_epi16(YMM0);
        _mm512_storeu_si512((__m512i *) &Sequence[x], ZMM1);

    }
}

void
InitializeTestingEnvironmentSequence(struct combined * CombinedData,
        struct u16_array * TestingEnvironment, u32 SizeOfCombiner)
{
    for (u32 i = 0; i < SizeOfCombiner; i++)
    {
        TestingEnvironment[i].uID = CombinedData[i].uID;
        CopyTheStringUsingSimd(CombinedData[i].sData,
                TestingEnvironment[i].uaSequence, (u16) 1024);
    }
}


void
PairFinder(u16 * Pair, struct pair_occurence * VocabDictionary, u16 PairArraySize)
{
    u8 PairSwitch = 0;

    for (u16 x = 0; x < PairArraySize; x++)
    {
        if (!VocabDictionary[x].uaPairs[0])
        {
            VocabDictionary[x].uaPairs[0] = Pair[0];
            VocabDictionary[x].uaPairs[1] = Pair[1];
            VocabDictionary[x].uOccurence++;
            break;
        }

        PairSwitch += VocabDictionary[x].uaPairs[0] == Pair[0];
        PairSwitch += VocabDictionary[x].uaPairs[1] == Pair[1];

        if (PairSwitch == 2)
        {
            VocabDictionary[x].uOccurence++;
            break;
        }

        PairSwitch = 0;
    }
}

void
PairCounter(u16 * Sequence, struct pair_occurence * VocabDictionary,
        u16 PairArraySize)
{
    u16 Pair[2];

    for (u16 i = 0; i < 1023; i++)
    {
        if (Sequence[i + 1] == 0)
        {
            break;
        }

        Pair[0] = Sequence[i];
        Pair[1] = Sequence[i+1];
        PairFinder(Pair, VocabDictionary, PairArraySize);
    }
}

void
VocabConstructor(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize)
{
    for (u32 x = 0; x < BPEEnvironmentArraySize; x++)
    {
        PairCounter(TestingEnvironment[x].uaSequence, VocabDictionary,
                PairArraySize);
    }
}

void
MaxVocabOccurence(struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u8 Index, struct vocab * Vocabs)
{
    u32 MaxOccurence = 0;
    u16 Counter = 0;

    for (u16 x = 0; x < PairArraySize; x++)
    {
        if (VocabDictionary[x].uOccurence > MaxOccurence)
        {
            Counter = x;
            MaxOccurence = VocabDictionary[x].uOccurence;
        }
    }

    Vocabs[Index].uaPairs[0] = VocabDictionary[Counter].uaPairs[0];
    Vocabs[Index].uaPairs[1] = VocabDictionary[Counter].uaPairs[1];

    Vocabs[Index].uPairID = Index + 1024 + 1;
}

void
ResetVocabDictionary(struct pair_occurence * VocabDictionary,
        u16 PairArraySize)
{
    for (u16 x = 0; x < PairArraySize; x+= 4)
    {
        VocabDictionary[x].uaPairs[0] = 0;
        VocabDictionary[x].uaPairs[1] = 0;
        VocabDictionary[x].uOccurence = 0;

        VocabDictionary[x + 1].uaPairs[0] = 0;
        VocabDictionary[x + 1].uaPairs[1] = 0;
        VocabDictionary[x + 1].uOccurence = 0;

        VocabDictionary[x + 2].uaPairs[0] = 0;
        VocabDictionary[x + 2].uaPairs[1] = 0;
        VocabDictionary[x + 2].uOccurence = 0;

        VocabDictionary[x + 3].uaPairs[0] = 0;
        VocabDictionary[x + 3].uaPairs[1] = 0;
        VocabDictionary[x + 3].uOccurence = 0;

    }
}

void
LeftShift2ByteArray(u16 * Array, u16 Index, u16 ArraySize)
{
    __m512i ZMM0;

    if (ArraySize - Index > 32)
    {
        u16 RemainingSize = ArraySize - Index - 1;
        u8 Remainder = RemainingSize % 32;
        u16 NewArraySize = ArraySize - Remainder - 1;

        for (u16 x = Index; x < NewArraySize; x+= 32)
        {
            ZMM0 = _mm512_loadu_si512((__m512i *) &Array[x+1]);
            _mm512_storeu_si512((__m512i *) &Array[x], ZMM0);
        }

        for (u16 x = NewArraySize; x < ArraySize - 1; x++)
        {
            Array[x] = Array[x + 1];
        }
    }
    else
    {
        for (u16 x = Index; x < ArraySize - 1; x++)
        {
            Array[x] = Array[x + 1];
        }
    }

    Array[ArraySize] = '\0';
}

void
SequenceCompression(struct vocab *Vocab, u16 * Sequence)
{
    u64 StartLeftShift = 0;
    u64 EndLeftShift = 0;

    for (u16 x = 0; x < 1023; x++)
    {
        if (Vocab->uaPairs[0] == Sequence[x] &&
                Vocab->uaPairs[1] == Sequence[x + 1])
        {
            Sequence[x] = Vocab->uPairID;
            Sequence[x + 1] = 7;
            
            StartLeftShift = read_cpu_timer();
            LeftShift2ByteArray(Sequence, x+1, 1024);
            EndLeftShift = read_cpu_timer() - StartLeftShift;
            LeftShift += EndLeftShift;
        }
    }
}

void
BPECompression(struct vocab *Vocab, u16 Index, u32 BPEEnvironmentArraySize,
        struct u16_array * TestingEnvironment)
{
    for (u32 x = 0; x < BPEEnvironmentArraySize; x++)
    {
        SequenceCompression(&Vocab[Index], TestingEnvironment[x].uaSequence);
    }
}

void
BPEImplementation(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, struct vocab * Vocabs, u8 VocabSize)
{
    ResetVocabDictionary(VocabDictionary, PairArraySize);
    for (u8 x = 0; x < VocabSize; x++)
    {
        VocabConstructor(TestingEnvironment, VocabDictionary, PairArraySize,
                BPEEnvironmentArraySize);
        MaxVocabOccurence(VocabDictionary, PairArraySize, x, Vocabs);
        ResetVocabDictionary(VocabDictionary, PairArraySize);
        BPECompression(Vocabs, x, BPEEnvironmentArraySize, TestingEnvironment);
    }
}

u64
ProfiledVocabConsructor(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize)
{
    u64 StartForConstructor;
    u64 EndForConstructor;
    StartForConstructor = read_cpu_timer();
    VocabConstructor(TestingEnvironment, VocabDictionary, PairArraySize,
            BPEEnvironmentArraySize);
    EndForConstructor = read_cpu_timer() - StartForConstructor;
    return EndForConstructor;
}

u64
ProfiledMaxVocabOccurence(struct pair_occurence * VocabDictionary,
        u16 PairArraySize, u16 Index, struct vocab * Vocabs)
{
    u64 StartForOccurence;
    u64 EndForOccurence;
    StartForOccurence = read_cpu_timer();
    MaxVocabOccurence(VocabDictionary, PairArraySize, Index, Vocabs);
    EndForOccurence = read_cpu_timer() - StartForOccurence;

    return EndForOccurence;
}

u64
ProfiledInsideReset(struct pair_occurence * VocabDictionary, u16 PairArraySize)
{
    u64 StartForInsideReset;
    u64 EndForInsideReset;
    StartForInsideReset = read_cpu_timer();
    ResetVocabDictionary(VocabDictionary, PairArraySize);
    EndForInsideReset = read_cpu_timer() - StartForInsideReset;
    return EndForInsideReset;
}

u64
ProfiledBPECompression(struct vocab * Vocabs, u16 Index,
        u32 BPEEnvironmentArraySize, struct u16_array * TestingEnvironment)
{
    u64 StartForCompression;
    u64 EndForCompression;
    StartForCompression = read_cpu_timer();
    BPECompression(Vocabs, Index, BPEEnvironmentArraySize, TestingEnvironment);
    EndForCompression = read_cpu_timer() - StartForCompression;
    return EndForCompression;
}

void
BPEImplementationProfiled(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, struct vocab * Vocabs, u8 VocabSize)
{
    u64 ClocksForEachFunction[5];

    u64 StartForOutsideReset = read_cpu_timer();
    ResetVocabDictionary(VocabDictionary, PairArraySize);
    u64 EndForOutsideReset = read_cpu_timer() - StartForOutsideReset;
    ClocksForEachFunction[0] = EndForOutsideReset;

    u64 CPUFREQ;

    for (u8 x = 0; x < VocabSize; x++)
    {
        ClocksForEachFunction[1] += ProfiledVocabConsructor(TestingEnvironment,
                VocabDictionary, PairArraySize, BPEEnvironmentArraySize);

        ClocksForEachFunction[2] += ProfiledMaxVocabOccurence(
                VocabDictionary, PairArraySize, x, Vocabs);

        CPUFREQ = estimate_cpu_timer_freq();

        ClocksForEachFunction[3] += ProfiledInsideReset(VocabDictionary,
                PairArraySize);

        CPUFREQ = estimate_cpu_timer_freq();

        ClocksForEachFunction[4] += ProfiledBPECompression(Vocabs, x,
                BPEEnvironmentArraySize, TestingEnvironment);
    }

    CPUFREQ = estimate_cpu_timer_freq();
    u64 TotalClocks = 0;
    for (u8 x = 0; x < 5; x++)
    {
        TotalClocks += ClocksForEachFunction[x];
    }

    printf("CPUFREQ: %llu\n", CPUFREQ);

    printf("Total clks(time): %llu", TotalClocks);
    printf(" (%.2fs)\n", (f64) TotalClocks / (f64) CPUFREQ);

    printf("Outside reset clks(time): %llu", ClocksForEachFunction[0]);
    printf(" (%.2fs)", (f64) ClocksForEachFunction[0] / (f64) CPUFREQ);
    printf(" %.2f%%\n", ((f64) ClocksForEachFunction[0] / (f64) TotalClocks) *
            100);

    printf("Vocab constructor clks(time): %llu", ClocksForEachFunction[1]);
    printf(" (%.2fs)", (f64) ClocksForEachFunction[1] / (f64) CPUFREQ);
    printf(" %.2f%%\n", ((f64) ClocksForEachFunction[1] / (f64) TotalClocks) *
            100);

    printf("Occurence clks(time): %llu", ClocksForEachFunction[2]);
    printf(" (%.2fs)", (f64) ClocksForEachFunction[2] / (f64) CPUFREQ);
    printf(" %.2f%%\n", ((f64) ClocksForEachFunction[2] / (f64) TotalClocks) *
            100);

    printf("Inside reset clks(time): %llu", ClocksForEachFunction[3]);
    printf(" (%.2fs)", (f64) ClocksForEachFunction[3] / (f64) CPUFREQ);
    printf(" %.2f%%\n", ((f64) ClocksForEachFunction[3] / (f64) TotalClocks) *
            100);

    printf("Compression clks(time): %llu", ClocksForEachFunction[4]);
    printf(" (%.2fs)", (f64) ClocksForEachFunction[4] / (f64) CPUFREQ);
    printf(" %.2f%%\n", ((f64) ClocksForEachFunction[4] / (f64) TotalClocks) *
            100);

    printf("\nShift array clks(time): %llu", LeftShift);
    printf(" (%.2fs)", (f64) LeftShift / (f64) CPUFREQ);
    printf(" %.2f%%\n", ((f64) LeftShift / (f64) TotalClocks) *
            100);
}

void
PairFinderMultiThread(u16 * Pair, struct pair_occurence * VocabDictionary, u16 PairArraySize)
{
    u8 PairSwitch = 0;

    for (u16 x = 0; x < PairArraySize; x++)
    {
        if (!VocabDictionary[x].uaPairs[0])
        {
            VocabDictionary[x].uaPairs[0] = Pair[0];
            VocabDictionary[x].uaPairs[1] = Pair[1];
            InterlockedIncrement(&VocabDictionary[x].uOccurence);
            break;
        }

        PairSwitch += VocabDictionary[x].uaPairs[0] == Pair[0];
        PairSwitch += VocabDictionary[x].uaPairs[1] == Pair[1];

        if (PairSwitch == 2)
        {
            InterlockedIncrement(&VocabDictionary[x].uOccurence);
            break;
        }

        PairSwitch = 0;
    }
}

void
PairCounterMultiThread(u16 * Sequence, struct pair_occurence * VocabDictionary,
        u16 PairArraySize)
{
    u16 Pair[2];

    for (u16 i = 0; i < 1023; i++)
    {
        if (Sequence[i + 1] == 0)
        {
            break;
        }

        Pair[0] = Sequence[i];
        Pair[1] = Sequence[i+1];
        PairFinderMultiThread(Pair, VocabDictionary, PairArraySize);
    }
}


void ThreadInitializerForVocabConstructor(
        struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, u8 TotalThreads,
        struct constructor_data * Parameters)
{
    u32 StepSize = BPEEnvironmentArraySize / TotalThreads;
    u8 Extra = BPEEnvironmentArraySize - (StepSize * TotalThreads);

    for (u8 x = 0; x < TotalThreads; x++)
    {
        Parameters[x].TestingEnvironment = TestingEnvironment;
        Parameters[x].VocabDictionary = VocabDictionary;
        Parameters[x].uPairArraySize = PairArraySize;
        Parameters[x].uStepSize = StepSize;
        Parameters[x].Extra = 0;
        Parameters[x].ThreadIndex = x;
    }

    Parameters[TotalThreads - 1].Extra = Extra;
}


DWORD WINAPI
VocabConstructorMultiThread(LPVOID lpParameter)
{
    constructor_data * Parameters = (constructor_data *) lpParameter;
    u32 StartIndex = Parameters -> uStepSize * Parameters -> ThreadIndex;
    u32 EndIndex = Parameters -> uStepSize + StartIndex + Parameters -> Extra;

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        PairCounterMultiThread(Parameters -> TestingEnvironment[x].uaSequence,
                Parameters -> VocabDictionary, Parameters -> uPairArraySize);
    }
    return 0;
}

void ThreadInitializerForVocabReset(
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u8 TotalThreads, struct vocab_dictionary_data * Parameters)
{
    u32 StepSize = PairArraySize / TotalThreads;
    u8 Extra = PairArraySize - (StepSize * TotalThreads);

    for (u8 x = 0; x < TotalThreads; x++)
    {
        Parameters[x].VocabDictionary = VocabDictionary;
        Parameters[x].uStepSize = StepSize;
        Parameters[x].Extra = 0;
        Parameters[x].ThreadIndex = x;
    }

    Parameters[TotalThreads - 1].Extra = Extra;
}

DWORD WINAPI
ResetVocabDictionaryMultiThread(LPVOID lpParameter)
{
    vocab_dictionary_data * Parameters = (vocab_dictionary_data *) lpParameter;
    u32 StartIndex = Parameters -> uStepSize * Parameters -> ThreadIndex;
    u32 EndIndex = Parameters -> uStepSize + StartIndex + Parameters -> Extra;

    for (u16 x = StartIndex; x < EndIndex; x+= 4)
    {
        Parameters -> VocabDictionary[x].uaPairs[0] = 0;
        Parameters -> VocabDictionary[x].uaPairs[1] = 0;
        Parameters -> VocabDictionary[x].uOccurence = 0;

        Parameters -> VocabDictionary[x + 1].uaPairs[0] = 0;
        Parameters -> VocabDictionary[x + 1].uaPairs[1] = 0;
        Parameters -> VocabDictionary[x + 1].uOccurence = 0;

        Parameters -> VocabDictionary[x + 2].uaPairs[0] = 0;
        Parameters -> VocabDictionary[x + 2].uaPairs[1] = 0;
        Parameters -> VocabDictionary[x + 2].uOccurence = 0;

        Parameters -> VocabDictionary[x + 3].uaPairs[0] = 0;
        Parameters -> VocabDictionary[x + 3].uaPairs[1] = 0;
        Parameters -> VocabDictionary[x + 3].uOccurence = 0;

    }
    return 0;
}

void ThreadInitializerForCompression(struct vocab *Vocab, u16 Index,
        u32 BPEEnvironmentArraySize, struct u16_array * TestingEnvironment,
        u8 TotalThreads, struct compressor_data * Parameters)
{
    u32 StepSize = BPEEnvironmentArraySize / TotalThreads;
    u8 Extra = BPEEnvironmentArraySize - (StepSize * TotalThreads);

    for (u8 x = 0; x < TotalThreads; x++)
    {
        Parameters[x].Vocab = Vocab;
        Parameters[x].Index = Index;
        Parameters[x].TestingEnvironment = TestingEnvironment;
        Parameters[x].uStepSize = StepSize;
        Parameters[x].Extra = 0;
        Parameters[x].ThreadIndex = x;
    }

    Parameters[TotalThreads - 1].Extra = Extra;
}

DWORD WINAPI
BPECompressionMultiThread(LPVOID lpParameter)
{
    compressor_data * Parameters = (compressor_data*) lpParameter;
    u32 StartIndex = Parameters -> uStepSize * Parameters -> ThreadIndex;
    u32 EndIndex = Parameters -> uStepSize + StartIndex + Parameters -> Extra;

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        SequenceCompression(&Parameters -> Vocab[Parameters -> Index],
                Parameters -> TestingEnvironment[x].uaSequence);
    }
    return 0;
}

void
SmallSequenceCopyUsingSimd(u16 * CompressedSequence, u16 ArraySize,
        f32 * SmallSequence)
{
    __m256i YMM0;
    __m512i ZMM1;

    for (u16 x = 0; x < ArraySize; x+=16)
    {
        YMM0 = _mm256_loadu_si256((__m256i *) &CompressedSequence[x]);
        ZMM1 = _mm512_cvtepi16_epi32(YMM0);
        ZMM1 = _mm512_cvtepi32_ps(ZMM1);
        _mm512_storeu_si512((__m512i *) &SmallSequence[x], ZMM1);
    }
}

void
SmallSequenceCompressionCopy(struct u16_array * TestingEnvironment,
        struct bpe_array * CompressedArray, u32 BPEEnvironmentArraySize)
{
    for (u32 x = 0; x < BPEEnvironmentArraySize; x++)
    {
        SmallSequenceCopyUsingSimd(TestingEnvironment[x].uaSequence, 512,
                CompressedArray[x].uaSequence);
    }
}

void
ThreadInitializeForCopyData(struct u16_array * TestingEnvironment,
        struct bpe_array * CompressedArray, u32 BPEEnvironmentArraySize,
        struct data_copy * Parameters, u8 TotalThreads)
{
    u32 StepSize = BPEEnvironmentArraySize / TotalThreads;
    u8 Extra = BPEEnvironmentArraySize - (StepSize * TotalThreads);

    for (u8 x = 0; x < TotalThreads; x++)
    {
        Parameters[x].TestingEnvironment = TestingEnvironment;
        Parameters[x].CompressedArray = CompressedArray;
        Parameters[x].uStepSize = StepSize;
        Parameters[x].Extra = 0;
        Parameters[x].ThreadIndex = x;
    }

    Parameters[TotalThreads - 1].Extra = Extra;
}

DWORD WINAPI
SmallSequenceCompressionCopyMultiThread(LPVOID lpParameter)
{
    data_copy * Parameters = (data_copy *) lpParameter;
    u32 StartIndex = Parameters -> uStepSize * Parameters -> ThreadIndex;
    u32 EndIndex = Parameters -> uStepSize + StartIndex + Parameters -> Extra;

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        SmallSequenceCopyUsingSimd(Parameters -> TestingEnvironment[x].uaSequence, 512,
                Parameters -> CompressedArray[x].uaSequence);
    }

    return 0;
}
