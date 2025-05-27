/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <cstdlib>
#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <windows.h>

#include "combiner.h"
#include "csv_parser.h"
#include "data_cleaning.h"
#include "repetition_tester.h"
#include "typedef.h"
#include "BPE.h"

#define MAX_NUM_THREADS 24
#define PROFILER 1

static thread_info ThreadInfo[MAX_NUM_THREADS];
static function_parameter_data ApexNexus[MAX_NUM_THREADS];
static combiner_threads CombinerWorkers[MAX_NUM_THREADS];
static constructor_data ApexConstructorParameters[MAX_NUM_THREADS];
static vocab_dictionary_data ApexVocabResetParameters[MAX_NUM_THREADS];
static compressor_data ApexCompressorParameters[MAX_NUM_THREADS];
static data_copy ApexDataCopyParameters[MAX_NUM_THREADS];

HANDLE Threads[MAX_NUM_THREADS];
DWORD ThreadID[MAX_NUM_THREADS];

static struct games gGamesTable;
static struct category gCategoryTable;
static struct genre gGenreTable;
static struct summary gSummaryTable;
static struct other gOtherTable;

static id_data GlobalIdData;

enum FileNames {
    GamesFile,
    CategoryFile,
    GenreFile,
    SummaryFile,
    OtherFile,
};


u8
GetCpuThreadCount()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

void *
GameParseInitialize(csv_info *Info, struct games *GamesTable)
{
    static struct games_columns *GamesData;
    u32 SizeArray = (Info->uSizeArray - 1) / Info->ubFirstLineFeed;
    GamesData =
        (struct games_columns *)malloc(sizeof(struct games_columns) * SizeArray);
    GamesTable->Data = GamesData;
    GamesTable->uSizeArray = SizeArray;
    void *Table = (void *)GamesTable;
    return Table;
}

void *
CategoryParseInitialize(csv_info *Info, struct category *CategoryTable)
{
    static struct category_column *CategoryData;
    u32 SizeArray = (Info->uSizeArray - 1) / Info->ubFirstLineFeed;
    CategoryData = (struct category_column *)malloc(
            sizeof(struct category_column) * SizeArray);
    CategoryTable->Data = CategoryData;
    CategoryTable->uSizeArray = SizeArray;
    void *Table = (void *)CategoryTable;
    return Table;
}

void *
GenreParseInitialize(csv_info *Info, struct genre *GenreTable)
{
    static struct genre_column *GenreData;
    u32 SizeArray = (Info->uSizeArray - 1) / Info->ubFirstLineFeed;
    GenreData =
        (struct genre_column *)malloc(sizeof(struct genre_column) * SizeArray);
    GenreTable->Data = GenreData;
    GenreTable->uSizeArray = SizeArray;
    void *Table = (void *)GenreTable;
    return Table;
}

void *
SummaryParseInitialize(csv_info *Info, struct summary *SummaryTable)
{
    static struct summary_column *SummaryData;
    u32 SizeArray = (Info->uSizeArray - 1) / Info->ubFirstLineFeed;
    SummaryData = (struct summary_column *)malloc(sizeof(struct summary_column) *
            SizeArray);
    SummaryTable->Data = SummaryData;
    SummaryTable->uSizeArray = SizeArray;
    void *Table = (void *)SummaryTable;
    return Table;
}

void *
OtherParseInitialize(csv_info *Info, struct other *OtherTable)
{
    static struct other_column *OtherData;
    u32 SizeArray = (Info->uSizeArray - 1) / Info->ubFirstLineFeed;
    OtherData =
        (struct other_column *)malloc(sizeof(struct other_column) * SizeArray);
    OtherTable->Data = OtherData;
    OtherTable->uSizeArray = SizeArray;
    void *Table = (void *)OtherTable;
    return Table;
}

void
GameParse(u32 SizeArray, char *buffer, u32 *SeveranceNexus)
{
    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, GamesParseMultiThread, &ThreadInfo[x],
                0, &ThreadID[x]);
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }

    GameColumnParse(&gGamesTable, SizeArray, buffer, SeveranceNexus);
}

void
CategoryParse(u32 SizeArray, char *buffer, u32 *SeveranceNexus)
{
    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CategoryParseMultiThread,
                &ThreadInfo[x], 0, &ThreadID[x]);
    }

    WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }

    CategoryColumnParse(&gCategoryTable, SizeArray, buffer, SeveranceNexus);
}

void
GenreParse(u32 SizeArray, char *buffer, u32 *SeveranceNexus)
{
    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, GenreParseMultiThread, &ThreadInfo[x],
                0, &ThreadID[x]);
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }

    GenreColumnParse(&gGenreTable, SizeArray, buffer, SeveranceNexus);
}

void
SummaryParse(u32 SizeArray, char *buffer, u32 *SeveranceNexus)
{
    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, SummaryParseMultiThread,
                &ThreadInfo[x], 0, &ThreadID[x]);
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
        CloseHandle(Threads[x]);
    }

    SummaryColumnParse(&gSummaryTable, SizeArray, buffer, SeveranceNexus);
}

void
OtherParse(u32 SizeArray, char *buffer, u32 *SeveranceNexus)
{
    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, OtherParseMultiThread, &ThreadInfo[x],
                0, &ThreadID[x]);
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
        CloseHandle(Threads[x]);
    }

    OtherColumnParse(&gOtherTable, SizeArray, buffer, SeveranceNexus);
}

void
FileParser(char *FileName, int TypeOfFile)
{
    FILE *ImmatureFilePtr = fopen(FileName, "rb");

    if (ImmatureFilePtr == NULL)
    {
        perror("File is not read");
    }

    struct stat Stat = FileSize(FileName);
    char *TransientBuffer;

    u32 *SeveranceNexus = (u32 *)malloc(sizeof(u32) * (1048576 * 2));
    TransientBuffer = (char *)malloc(Stat.st_size);

    if (TransientBuffer == NULL)
    {
        printf("can't allocate buffer for %s\n", FileName);
    }

    fread(TransientBuffer, sizeof(char), Stat.st_size, ImmatureFilePtr);

    csv_info TmpInfo = {};
    struct csv_info *Info = FindValuesFromBuffer(TransientBuffer, SeveranceNexus,
            Stat.st_size, &TmpInfo);

    u32 SizeArray = Info->uSizeArray;
    void *pDisjointedFlowTable;
    /*if (TypeOfFile == GamesFile) {
      pDisjointedFlowTable = GameParseInitialize(*Info, &gGamesTable);
      } else if (TypeOfFile == CategoryFile) {
      pDisjointedFlowTable = CategoryParseInitialize(*Info, &gCategoryTable);
      } else if (TypeOfFile == GenreFile) {
      pDisjointedFlowTable = GenreParseInitialize(*Info, &gGenreTable);
      } else if (TypeOfFile == SummaryFile) {
      pDisjointedFlowTable = SummaryParseInitialize(*Info, &gSummaryTable);
      } else if (TypeOfFile == OtherFile) {
      pDisjointedFlowTable = OtherParseInitialize(*Info, &gOtherTable);
      }*/

    switch (TypeOfFile)
    {
        case GamesFile:
            pDisjointedFlowTable = GameParseInitialize(Info, &gGamesTable);
            break;
        case CategoryFile:
            pDisjointedFlowTable = CategoryParseInitialize(Info, &gCategoryTable);
            break;
        case GenreFile:
            pDisjointedFlowTable = GenreParseInitialize(Info, &gGenreTable);
            break;
        case SummaryFile:
            pDisjointedFlowTable = SummaryParseInitialize(Info, &gSummaryTable);
            break;
        case OtherFile:
            pDisjointedFlowTable = OtherParseInitialize(Info, &gOtherTable);
            break;
    }

    ThreadInitializer(pDisjointedFlowTable, TransientBuffer, SeveranceNexus,
            MAX_NUM_THREADS, ThreadInfo, Info);

    /*if (TypeOfFile == GamesFile) {
      GameParse(SizeArray, TransientBuffer, SeveranceNexus,
      pDisjointedFlowTable); } else if (TypeOfFile == CategoryFile) {
      CategoryParse(SizeArray, TransientBuffer, SeveranceNexus,
      pDisjointedFlowTable);
      }*/

    switch (TypeOfFile)
    {
        case GamesFile:
            GameParse(SizeArray, TransientBuffer, SeveranceNexus);
            break;
        case CategoryFile:
            CategoryParse(SizeArray, TransientBuffer, SeveranceNexus);
            break;
        case GenreFile:
            GenreParse(SizeArray, TransientBuffer, SeveranceNexus);
            break;
        case SummaryFile:
            SummaryParse(SizeArray, TransientBuffer, SeveranceNexus);
            break;
        case OtherFile:
            OtherParse(SizeArray, TransientBuffer, SeveranceNexus);
            break;
    }
}

void
GameTableRecordingMultiThread(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    //SpecialThreads[MAX_NUM_THREADS];
    //DWORD SpecialThreadID[MAX_NUM_THREADS];

    void *vpGameTable = (void *)&gGamesTable;

    InitializeMultiThreadFunctionParameter(vpGameTable, MAX_NUM_THREADS,
            GlobalIdDetails, GlobalIdData,
            gGamesTable.uSizeArray, ApexNexus);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CheckGamesTableMultiThread,
                &ApexNexus[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
CategoryTableRecordingMultiThread(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    //HANDLE SpecialThreads[MAX_NUM_THREADS];
    //DWORD SpecialThreadID[MAX_NUM_THREADS];

    void *vpCategoryTable = (void *)&gCategoryTable;

    InitializeMultiThreadFunctionParameter(vpCategoryTable, MAX_NUM_THREADS,
            GlobalIdDetails, GlobalIdData,
            gCategoryTable.uSizeArray, ApexNexus);

    //printf("Id data size: %u\n", GlobalIdData->uSize);
    //printf("Size of Category Table: %u\n", gCategoryTable.uSizeArray);
    //printf("Thread: %u Stepsize %u\n", ApexNexus[0].uStepSize, ApexNexus[0].ThreadIndex);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CheckCategoryTableMultiThread,
                &ApexNexus[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
GenreTableRecordingMultiThread(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    //HANDLE SpecialThreads[MAX_NUM_THREADS];
    //DWORD SpecialThreadID[MAX_NUM_THREADS];

    void *vpGenreTable = (void *)&gGenreTable;

    InitializeMultiThreadFunctionParameter(vpGenreTable, MAX_NUM_THREADS,
            GlobalIdDetails, GlobalIdData,
            gGenreTable.uSizeArray, ApexNexus);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CheckGenreTableMultiThread,
                &ApexNexus[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
SummaryTableRecordingMultiThread(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    //HANDLE SpecialThreads[MAX_NUM_THREADS];
    //DWORD SpecialThreadID[MAX_NUM_THREADS];

    void *vpSummaryTable = (void *)&gSummaryTable;

    InitializeMultiThreadFunctionParameter(vpSummaryTable, MAX_NUM_THREADS,
            GlobalIdDetails, GlobalIdData,
            gSummaryTable.uSizeArray, ApexNexus);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CheckSummaryTableMultiThread,
                &ApexNexus[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
OtherTableRecordingMultiThread(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    //HANDLE SpecialThreads[MAX_NUM_THREADS];
    //DWORD SpecialThreadID[MAX_NUM_THREADS];

    void *vpOtherTable = (void *)&gOtherTable;

    InitializeMultiThreadFunctionParameter(vpOtherTable, MAX_NUM_THREADS,
            GlobalIdDetails, GlobalIdData,
            gOtherTable.uSizeArray, ApexNexus);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CheckOtherTableMultiThread,
                &ApexNexus[x], 0, &ThreadID[x]);
        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
PerformMultiThreadRecording(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    GameTableRecordingMultiThread(GlobalIdDetails, GlobalIdData);
    CategoryTableRecordingMultiThread(GlobalIdDetails, GlobalIdData);
    GenreTableRecordingMultiThread(GlobalIdDetails, GlobalIdData);
    SummaryTableRecordingMultiThread(GlobalIdDetails, GlobalIdData);
    OtherTableRecordingMultiThread(GlobalIdDetails, GlobalIdData);
}


void
GameCombineInitializeAndExecuteMultiThread(u32 TotalRowsOfCombiner,
        struct combined * CombinedData)
{
    void * vpTable = (void *) &gGamesTable;

    ThreadInitializerForCombiner(vpTable, MAX_NUM_THREADS, CombinedData,
            gGamesTable.uSizeArray, CombinerWorkers, TotalRowsOfCombiner);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, GameCombineMultiThread,
                &CombinerWorkers[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
CategoryCombineInitializeAndExecuteMultiThread(u32 TotalRowsOfCombiner,
        struct combined * CombinedData)
{
    void * vpTable = (void *) &gCategoryTable;

    ThreadInitializerForCombiner(vpTable, MAX_NUM_THREADS, CombinedData,
            gCategoryTable.uSizeArray, CombinerWorkers, TotalRowsOfCombiner);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, CategoryCombineMultiThread,
                &CombinerWorkers[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
GenreCombineInitializeAndExecuteMultiThread(u32 TotalRowsOfCombiner,
        struct combined * CombinedData)
{
    void * vpTable = (void *) &gGenreTable;

    ThreadInitializerForCombiner(vpTable, MAX_NUM_THREADS, CombinedData,
            gGenreTable.uSizeArray, CombinerWorkers, TotalRowsOfCombiner);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, GenreCombineMultiThread,
                &CombinerWorkers[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
SummaryCombineInitializeAndExecuteMultiThread(u32 TotalRowsOfCombiner,
        struct combined * CombinedData)
{
    void * vpTable = (void *) &gSummaryTable;

    ThreadInitializerForCombiner(vpTable, MAX_NUM_THREADS, CombinedData,
            gSummaryTable.uSizeArray, CombinerWorkers, TotalRowsOfCombiner);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, SummaryCombineMultiThread,
                &CombinerWorkers[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
OtherCombineInitializeAndExecuteMultiThread(u32 TotalRowsOfCombiner,
        struct combined * CombinedData)
{
    void * vpTable = (void *) &gOtherTable;

    ThreadInitializerForCombiner(vpTable, MAX_NUM_THREADS, CombinedData,
            gOtherTable.uSizeArray, CombinerWorkers, TotalRowsOfCombiner);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, OtherCombineMultiThread,
                &CombinerWorkers[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
BPEVocabConstructorMultiThread(struct u16_array * TestingEnvironment,
        struct pair_occurence * VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize)
{
    ThreadInitializerForVocabConstructor(TestingEnvironment, VocabDictionary,
            PairArraySize, BPEEnvironmentArraySize, MAX_NUM_THREADS,
            ApexConstructorParameters);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, VocabConstructorMultiThread,
                &ApexConstructorParameters[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
PerformMultiThreadCombiner(struct combined * CombinedData,
        u32 TotalRowsOfCombiner, struct id_details * GlobalIdDetails,
        struct id_data * GlobalIdData)
{
    InitializeCombiner(CombinedData, GlobalIdDetails, GlobalIdData);

    GameCombineInitializeAndExecuteMultiThread(TotalRowsOfCombiner,
            CombinedData);

    CategoryCombineInitializeAndExecuteMultiThread(TotalRowsOfCombiner,
            CombinedData);

    GenreCombineInitializeAndExecuteMultiThread(TotalRowsOfCombiner,
            CombinedData);

    SummaryCombineInitializeAndExecuteMultiThread(TotalRowsOfCombiner,
            CombinedData);

    OtherCombineInitializeAndExecuteMultiThread(TotalRowsOfCombiner,
            CombinedData);
}


void
BPEVocabResetMultiThread(struct pair_occurence * VocabDictionary,
        u16 PairArraySize)
{
    ThreadInitializerForVocabReset(VocabDictionary, PairArraySize,
            MAX_NUM_THREADS, ApexVocabResetParameters);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, ResetVocabDictionaryMultiThread,
                &ApexVocabResetParameters[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
BPECompressionMultiThreadProcessor(struct u16_array * TestingEnvironment,
        u16 PairArraySize, u32 BPEEnvironmentArraySize, struct vocab * Vocab,
        u16 Index)
{
    ThreadInitializerForCompression(Vocab, Index, BPEEnvironmentArraySize,
            TestingEnvironment, MAX_NUM_THREADS, ApexCompressorParameters);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0, BPECompressionMultiThread,
                &ApexCompressorParameters[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}

void
BPEImplementationMultiThreadProfiled(struct u16_array *TestingEnvironment,
        struct pair_occurence *VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, struct vocab *Vocabs, u8 VocabSize)
{
    u64 StartForConstructor;
    u64 EndForConstructor;
    u64 StartForOccurence;
    u64 EndForOccurence;
    u64 StartForInsideReset;
    u64 EndForInsideReset;
    u64 StartForCompression;
    u64 EndForCompression;

    u64 ClocksForEachFunction[5];

    u64 StartForOutsideReset = read_cpu_timer();
    BPEVocabResetMultiThread(VocabDictionary, PairArraySize);
    u64 EndForOutsideReset = read_cpu_timer() - StartForOutsideReset;
    ClocksForEachFunction[0] = EndForOutsideReset;

    for (u8 x = 0; x < VocabSize; x++)
    {
        StartForConstructor = read_cpu_timer();
        BPEVocabConstructorMultiThread(TestingEnvironment, VocabDictionary,
                PairArraySize, BPEEnvironmentArraySize);
        EndForConstructor = read_cpu_timer() - StartForConstructor;
        ClocksForEachFunction[1] += EndForConstructor;

        StartForOccurence = read_cpu_timer();
        MaxVocabOccurence(VocabDictionary, PairArraySize, x, Vocabs);
        EndForOccurence = read_cpu_timer() - StartForOccurence;
        ClocksForEachFunction[2] += EndForOccurence;

        StartForInsideReset = read_cpu_timer();
        BPEVocabResetMultiThread(VocabDictionary, PairArraySize);
        EndForInsideReset = read_cpu_timer() - StartForInsideReset;
        ClocksForEachFunction[3] += EndForInsideReset;

        StartForCompression = read_cpu_timer();
        BPECompressionMultiThreadProcessor(TestingEnvironment, PairArraySize,
                BPEEnvironmentArraySize, Vocabs, x);
        EndForCompression = read_cpu_timer() - StartForCompression;
        ClocksForEachFunction[4] += EndForCompression;
    }
    u64 CPUFREQ = estimate_cpu_timer_freq();
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
}

void
BPEImplementationMultiThread(struct u16_array *TestingEnvironment,
        struct pair_occurence *VocabDictionary, u16 PairArraySize,
        u32 BPEEnvironmentArraySize, struct vocab *Vocabs, u8 VocabSize)
{
    BPEVocabResetMultiThread(VocabDictionary, PairArraySize);
    for (u8 x = 0; x < VocabSize; x++)
    {
        BPEVocabConstructorMultiThread(TestingEnvironment, VocabDictionary,
                PairArraySize, BPEEnvironmentArraySize);
        MaxVocabOccurence(VocabDictionary, PairArraySize, x, Vocabs);
        BPEVocabResetMultiThread(VocabDictionary, PairArraySize);
        BPECompressionMultiThreadProcessor(TestingEnvironment, PairArraySize,
                BPEEnvironmentArraySize, Vocabs, x);
    }
}


void
BPECompressionCopy(struct u16_array * TestingEnvironment,
        struct bpe_array * CompressedArray, u32 BPEEnvironmentArraySize)
{
    ThreadInitializeForCopyData(TestingEnvironment, CompressedArray,
            BPEEnvironmentArraySize, ApexDataCopyParameters,
            MAX_NUM_THREADS);

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        Threads[x] = CreateThread(0, 0,
                SmallSequenceCompressionCopyMultiThread,
                &ApexDataCopyParameters[x], 0, &ThreadID[x]);

        if (Threads[x] == NULL)
        {
            printf("Failed to created thread %u: %lu\n", x, GetLastError());
        }
    }

    DWORD Result = WaitForMultipleObjects(MAX_NUM_THREADS, Threads, TRUE, INFINITE);

    if (Result == WAIT_FAILED)
    {
        printf("WaitForMultipleObjects failed in GameTable, Error: %lu\n",
                GetLastError());
    }

    for (u8 x = 0; x < MAX_NUM_THREADS; x++)
    {
        CloseHandle(Threads[x]);
    }
}


int
main()
{
    u64 StartMultiThreadGames = read_cpu_timer();
    FileParser("/Data/games.csv", GamesFile);
    u64 EndMultiThreadGames = read_cpu_timer() - StartMultiThreadGames;

    u64 StartMultiThreadCategory = read_cpu_timer();
    FileParser("/Data/categories.csv", CategoryFile);
    u64 EndMultiThreadCategory = read_cpu_timer() - StartMultiThreadCategory;

    u64 StartMultiThreadGenre = read_cpu_timer();
    FileParser("/Data/genres.csv", GenreFile);
    u64 EndMultiThreadGenre = read_cpu_timer() - StartMultiThreadGenre;

    u64 StartMultiThreadSummary = read_cpu_timer();
    FileParser("/Data/descriptions.csv", SummaryFile);
    u64 EndMultiThreadSummary = read_cpu_timer() - StartMultiThreadSummary;

    u64 StartMultiThreadOther = read_cpu_timer();
    FileParser("/Data/steamspy_insights.csv", OtherFile);
    u64 EndMultiThreadOther = read_cpu_timer() - StartMultiThreadOther;

    static struct combined *pAllDataCombined;
    u32 SizeArray = gGamesTable.uSizeArray;

    static struct id_details *pGlobalIdDetails;
    pGlobalIdDetails = (id_details *) malloc(sizeof(id_details)
            * gGamesTable.uSizeArray);

    InitializeGlobalIdDetails(&gGamesTable, pGlobalIdDetails, &GlobalIdData);
    PerformMultiThreadRecording(pGlobalIdDetails, &GlobalIdData);

    /*CompareAllData(
      &gGamesTable,
      &gCategoryTable,
      &gGenreTable,
      &gSummaryTable,
      &gOtherTable,
      pGlobalIdDetails,
      &GlobalIdData);*/

    u32 PerfectScoreSize = CountOfPerfectScore(pGlobalIdDetails, &GlobalIdData);

    pAllDataCombined =
        (struct combined *)malloc(sizeof(struct combined) * PerfectScoreSize);
    if (pAllDataCombined == NULL) {
        printf("Pointer is null\n");
    }

    //printf("%u\n", PerfectScoreSize);

    /*CombineIntoString(
            pAllDataCombined,
            &gGamesTable,
            &gCategoryTable,
            &gGenreTable,
            &gSummaryTable,
            &gOtherTable,
            pGlobalIdDetails,
            &GlobalIdData,
            PerfectScoreSize);*/

    PerformMultiThreadCombiner(pAllDataCombined, PerfectScoreSize,
            pGlobalIdDetails, &GlobalIdData);

    free(gGamesTable.Data);
    free(gCategoryTable.Data);
    free(gGenreTable.Data);
    free(gSummaryTable.Data);
    free(gOtherTable.Data);

    //printf("%s\n", pAllDataCombined[0].sData);
    // printf("%llu\n", sizeof(pAllDataCombined[1].sData));

    struct u16_array * BPETestingEnvironment = (u16_array *) malloc(
            PerfectScoreSize * sizeof(u16_array));

    InitializeTestingEnvironmentSequence(pAllDataCombined,
            BPETestingEnvironment, PerfectScoreSize);

    u32 CounterNonCompressed = 0;
    for (u32  i = 0; i < 1024; i++)
    {
        if (BPETestingEnvironment[100].uaSequence[i] == 0)
        {
            break;
        }
        CounterNonCompressed++;
    }

/*    for (u16 x = 0; x < 1024; x++)
    {
        printf("%u ", BPETestingEnvironment[0].uaSequence[x]);
        if (x % 20 == 0)
        {
            printf("\n");
        }
    }*/

    u16 DictionarySize = 60000;
    struct pair_occurence * VocabDictionary = (struct pair_occurence *) malloc(
            DictionarySize * sizeof(pair_occurence));

    u8 VocabSize = 50;
    struct vocab * Vocabs = (struct vocab *) malloc(VocabSize * sizeof(vocab));

    /*BPEImplementationProfiled(BPETestingEnvironment, VocabDictionary, DictionarySize,
            PerfectScoreSize, Vocabs, VocabSize);*/
    BPEImplementationMultiThreadProfiled(BPETestingEnvironment, VocabDictionary, DictionarySize,
            PerfectScoreSize, Vocabs, VocabSize);

    /*for (u16 x = 0; x < 1024; x++)
    {
        printf("%u ", BPETestingEnvironment[100].uaSequence[x]);
        if (x % 20 == 0)
        {
            printf("\n");
        }
    }*/

    /*u32 CounterCompressed = 0;
    for (u32  i = 0; i < 1024; i++)
    {
        if (BPETestingEnvironment[100].uaSequence[i] == 0)
        {
            break;
        }
        CounterCompressed++;
    }
    for (u16 x = 0; x < DictionarySize; x++)
    {
        if (VocabDictionary[x].uaPairs[0] == 4 &&
                VocabDictionary[x].uaPairs[1] == 0)
        {
            printf("First value: %u\n", VocabDictionary[x].uaPairs[0]);
            printf("Second value: %u\n", VocabDictionary[x].uaPairs[1]);
            printf("%u\n", VocabDictionary[x].uOccurence);
            printf("%u\n", x);
        }
    }
    printf("First value: %u\n", Vocabs[0].uaPairs[0]);
    printf("Second value: %u\n", Vocabs[0].uaPairs[1]);
    printf("Non Compressed value: %u\n", CounterNonCompressed);
    printf("Compressed value: %u\n", CounterCompressed);
    printf("Percentage of orignal length: %.2f%%\n", ((f64) CounterCompressed /
            (f64) CounterNonCompressed) * 100);*/

    bpe_array * CompressedArray = (bpe_array *) malloc(sizeof(bpe_array) *
            PerfectScoreSize);
    BPECompressionCopy(BPETestingEnvironment, CompressedArray,
            PerfectScoreSize);

    for (u16 x = 0; x < 512; x++)
    {
        printf("%d ", pAllDataCombined[1].sData[x]);
        if (x % 20 == 0)
        {
            printf("\n");
        }
    }

    free(pGlobalIdDetails);
    free(pAllDataCombined);

    for (u16 x = 0; x < 512; x++)
    {
        printf("%u ", BPETestingEnvironment[1].uaSequence[x]);
        if (x % 20 == 0)
        {
            printf("\n");
        }
    }

    /*for (u16 x = 0; x < 512; x++)
    {
        printf("%.1f ", CompressedArray[1].uaSequence[x]);
        if (x % 20 == 0)
        {
            printf("\n");
        }
    }*/
    for (u8 x = 0; x < 50; x++)
    {
        printf("%u\n", Vocabs[x].uPairID);
        printf("%u\n", Vocabs[x].uaPairs[0]);
        printf("%u\n", Vocabs[x].uaPairs[1]);
    }

    //free(CompressedArray);

    /*u64 CPUFREQ = estimate_cpu_timer_freq();
      f64 MultiThreadTimeGames = (f64) EndMultiThreadGames / (f64) CPUFREQ;
      f64 MultiThreadTimeCategory = (f64) EndMultiThreadCategory / (f64) CPUFREQ;
      f64 MultiThreadTimeGenre = (f64) EndMultiThreadGenre / (f64) CPUFREQ;
      f64 MultiThreadTimeSummary = (f64) EndMultiThreadSummary / (f64) CPUFREQ;
      f64 MultiThreadTimeOther = (f64) EndMultiThreadOther / (f64) CPUFREQ;

      printf("MultiThread:\n%llu clks    (%.2fs)\n", EndMultiThreadGames,
      MultiThreadTimeGames);
      printf("MultiThread:\n%llu clks    (%.2fs)\n", EndMultiThreadCategory,
      MultiThreadTimeCategory);
      printf("MultiThread:\n%llu clks    (%.2fs)\n", EndMultiThreadGenre,
      MultiThreadTimeGenre);
      printf("MultiThread:\n%llu clks    (%.2fs)\n", EndMultiThreadSummary,
      MultiThreadTimeSummary);
      printf("MultiThread:\n%llu clks    (%.2fs)\n", EndMultiThreadOther,
      MultiThreadTimeOther);*/

    /*printf("%u\n", gGamesTable.Data[0].uID);
      printf("%s\n", gGamesTable.Data[0].sName);
      printf("%s\n", gGamesTable.Data[0].sReleaseDate);
      printf("%u\n", gGamesTable.Data[0].ubIsFree);
      printf("%s\n", gGamesTable.Data[0].sType);

      printf("%u\n", gCategoryTable.Data[0].uID);
      printf("%s\n", gCategoryTable.Data[0].sCategory);

      printf("%u\n", gGenreTable.Data[0].uID);
      printf("%s\n", gGenreTable.Data[0].sGenre);

      printf("%u\n", gSummaryTable.Data[0].uID);
      printf("%s\n", gSummaryTable.Data[0].sSummary);

      printf("%u\n", gOtherTable.Data[1].uID);
      printf("%s\n", gOtherTable.Data[1].sDeveloper);
      printf("%s\n", gOtherTable.Data[1].sPublisher);
      printf("%.2f\n", gOtherTable.Data[1].fPrice);
      printf("%s\n", gOtherTable.Data[1].sLanguage);*/

    /*printf("%u\n", gGamesTable.uSizeArray);
      printf("%u\n", gCategoryTable.uSizeArray);
      printf("%u\n", gGenreTable.uSizeArray);
      printf("%u\n", gSummaryTable.uSizeArray);
      printf("%u\n", gOtherTable.uSizeArray);*/


    return 0;
}
