/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#ifndef W__CODE_BPE_H_
#define W__CODE_BPE_H_

#include <windows.h>
#include "typedef.h"

struct bpe_array {
   f32 faSequence[512]; 
   u32 uID;
};

struct u16_array {
    u16 uaSequence[1024];
    u32 uID;
};

struct pair_occurence {
    u16 uaPairs[2];
    u32 uOccurence;
};

struct vocab{
    u16 uaPairs[2];
    u16 uPairID;
};

struct constructor_data{
    struct u16_array * TestingEnvironment;
    struct pair_occurence * VocabDictionary;
    u16 uPairArraySize;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
};

struct vocab_dictionary_data{
    struct pair_occurence * VocabDictionary;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
};

struct compressor_data{
    struct vocab * Vocab;
    u16 Index;
    struct u16_array * TestingEnvironment;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
};

struct data_copy{
    struct u16_array * TestingEnvironment;
    struct bpe_array * CompressedArray;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
};

void
InitializeTestingEnvironmentSequence(struct combined * CombinedData,
        struct u16_array * TestingEnvironment, u32 SizeOfCombiner);

void
MaxVocabOccurence(struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u8 Index, struct vocab * Vocabs);

void
BPEImplementation(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, struct vocab * Vocabs, u8 VocabSize);

void
BPEImplementationProfiled(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, struct vocab * Vocabs, u8 VocabSize);

void ThreadInitializerForVocabConstructor(
        struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, u8 TotalThreads,
        struct constructor_data * Parameters);

DWORD WINAPI
VocabConstructorMultiThread(LPVOID lpParameter);

void ThreadInitializerForVocabReset(
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u8 TotalThreads, struct vocab_dictionary_data * Parameters);

DWORD WINAPI
ResetVocabDictionaryMultiThread(LPVOID lpParameter);

void ThreadInitializerForCompression(struct vocab *Vocab, u16 Index,
        u32 BPEEnvironmentArraySize, struct u16_array * TestingEnvironment,
        u8 TotalThreads, struct compressor_data * Parameters);

DWORD WINAPI
BPECompressionMultiThread(LPVOID lpParameter);

void
SmallSequenceCompressionCopy(struct u16_array * TestingEnvironment,
        struct bpe_array * CompressedArray, u32 BPEEnvironmentArraySize);

void
ThreadInitializeForCopyData(struct u16_array * TestingEnvironment,
        struct bpe_array * CompressedArray, u32 BPEEnvironmentArraySize,
        struct data_copy * Parameters, u8 TotalThreads);

DWORD WINAPI
SmallSequenceCompressionCopyMultiThread(LPVOID lpParameter);

#endif // W__CODE_BPE_H_
