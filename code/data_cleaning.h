/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#ifndef W__CODE_DATA_CLEANING_H
#define W__CODE_DATA_CLEANING_H

#include "typedef.h"
#include "csv_parser.h"
#include <windows.h>
#include <processthreadsapi.h>

struct id_details {
    u32 uID;
    u8 ubOccurence;
};

struct id_data {
    struct id_details * Details;
    u32 uSize;
};

struct function_parameter_data {
    void * table;
    struct id_details * GlobalIdDetails;
    struct id_data * GlobalIdData;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
};

void
InitializeGlobalIdDetails(struct games * GameTable,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData);

void
CompareAllData(struct games * GameTable, struct category * CategoryTable,
        struct genre * GenreTable, struct summary * SummaryTable,
        struct other * OtherTable, struct id_details * GlobalIdDetails,
        struct id_data * GlobalIdData);

void
InitializeMultiThreadFunctionParameter(void *table, u8 TotalThreads,
        struct id_details *GlobalIdDetails, struct id_data *GlobalIdData,
        u32 SizeArray, struct function_parameter_data *Nexus);

DWORD WINAPI
CheckGamesTableMultiThread(LPVOID lpParameter);

DWORD WINAPI
CheckCategoryTableMultiThread(LPVOID lpParameter);

DWORD WINAPI
CheckGenreTableMultiThread(LPVOID lpParameter);

DWORD WINAPI
CheckSummaryTableMultiThread(LPVOID lpParameter);

DWORD WINAPI
CheckOtherTableMultiThread(LPVOID lpParameter);

#endif // !W__CODE_DATA_CLEANING_H
