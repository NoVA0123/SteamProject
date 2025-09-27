/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#ifndef W__CODE_COMBINER_H_
#define W__CODE_COMBINER_H_

#include "csv_parser.h"
#include "typedef.h"

#define MAX_COLUMN_SCORE 14

struct combined {
  u32 uID;
  char sData[1024];
};

u32
CountOfPerfectScore(struct id_details *GlobalIdDetails,
                        struct id_data *GlobalIdData);

void
CombineIntoString(struct combined *CombinedData, struct games *GameTable,
                       struct category *CategoryTable, struct genre *GenreTable,
                       struct summary *SummaryTable, struct other *OtherTable,
                       struct id_details *GlobalIdDetails,
                       struct id_data *GlobalIdData, u32 TotalRowsOfCombiner);

struct combiner_threads{
    void * table;
    struct combined * CombinedData;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
    u32 TotalRowsOfCombiner;
};

struct other_combiner_threads{
    struct other * OtherTable;
    struct combined * CombinedData;
    u32 uStepSize;
    u32 TotalRowsOfCombiner;
    u8 Extra;
    u8 TotalThreads;
};

void
InitializeCombiner(struct combined *CombinedData,
        struct id_details *GlobalIdDetails, struct id_data *GlobalIdData);

void
ThreadInitializerForCombiner(void * table, u8 TotalThreads,
        struct combined * CombinedData, u32 SizeArray,
        struct combiner_threads* Workers, u32 TotalRowsOfCombiner);

struct other_combiner_threads
SendDataForCombiner(struct other * OtherTable, struct combined * CombinedData,
        u32 SizeArray, u32 TotalRowsOfCombiner, u8 TotalThreads);

struct other_column *
GetDataForCombiner();

u32
GetTotalRowsOfCombiner();

DWORD WINAPI
GameCombineMultiThread(LPVOID lpParameter);

DWORD WINAPI
CategoryCombineMultiThread(LPVOID lpParameter);

DWORD WINAPI
GenreCombineMultiThread(LPVOID lpParameter);

DWORD WINAPI
SummaryCombineMultiThread(LPVOID lpParameter);

/*DWORD WINAPI
OtherCombineMultiThread(LPVOID lpParameter);*/

DWORD WINAPI
OtherCombineMultiThreadTest(LPVOID lpParameter);

#endif // !W__CODE_COMBINER_H_
