/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include "combiner.h"
#include "csv_parser.h"
#include "data_cleaning.h"
#include "typedef.h"
#include <math.h>
#include <stdio.h>
#include <windows.h>

void
CopyTheStringInCombiner(char *CombineBuffer, char *DataBuffer, u16 Size)
{
    u16 Index = 0;
    for (u16 x = 0; x < 1024; x++)
    {
        if (CombineBuffer[x] == '\0')
        {
            CombineBuffer[x] = ' ';
            for (u16 i = 0; i < Size; i++)
            {
                if (DataBuffer[i] == '\0')
                {
                    Index = i + 1;
                    break;
                }
                CombineBuffer[x + i + 1] = DataBuffer[i];
            }
            CombineBuffer[x + Index] = '\0';
            break;
        }
    }
}

void
ConvertIntToTheString(char *CombineBuffer, u32 Value)
{
    char buffer[10];
    u8 NulIndex = 0;
    u32 Quotient;
    u32 Remainder;
    u8 IfNotZero = 1;

    if (Value == 0)
    {
        IfNotZero = 0;
    }
    else
    {
        for (u8 x = 0; x < 10; x++)
        {
            if (Value == 0)
            {
                NulIndex = x;
                buffer[x] = '\0';
                break;
            }

            Quotient = Value / 10;
            Remainder = Value - (Quotient * 10);
            buffer[x] = (char)Remainder + '0';
            Value /= 10;
        }
    }

    for (u16 x = 0; x < 1024; x++)
    {
        if (CombineBuffer[x] == '\0')
        {
            CombineBuffer[x] = ' ';
            if (!IfNotZero)
            {
                CombineBuffer[x + 1] = '0';
                CombineBuffer[x + 2] = '\0';
                break;
            } else
            {
                for (u16 i = 0; i < NulIndex; i++)
                {
                    CombineBuffer[x + i + 1] = buffer[NulIndex - i - 1];
                }
            }

            CombineBuffer[x + NulIndex + 1] = '\0';
            break;
        }
    }
}

void
ConvertFloatIntoString(char * CombinedDataBuffer, u64 FloatValue)
{
    char buffer[10];
    u8 NulIndex = 0;
    u32 Quotient;
    u32 Remainder;
    u8 IfNotZero = 1;

    if (FloatValue == 0)
    {
        IfNotZero = 0;
    }
    else
    {
        for (u8 x = 0; x < 10; x++)
        {
            if (FloatValue == 0)
            {
                NulIndex = x;
                buffer[x] = '\0';
                break;
            }

            Quotient = FloatValue / 10;
            Remainder = FloatValue - (Quotient * 10);
            buffer[x] = (char)Remainder + '0';
            FloatValue /= 10;
        }
    }

    if (!NulIndex)
    {
        char TmpBuffer1 = buffer[NulIndex - 2]; 
        char TmpBuffer2 = buffer[NulIndex - 1]; 
        buffer[NulIndex - 2] = '.';
        buffer[NulIndex - 1] = TmpBuffer1;
        buffer[NulIndex] = TmpBuffer2;
        NulIndex++;
    }

    for (u16 x = 0; x < 1024; x++)
    {
        if (CombinedDataBuffer[x] == '\0')
        {
            CombinedDataBuffer[x] = ' ';
            if (!IfNotZero)
            {
                CombinedDataBuffer[x + 1] = '0';
                CombinedDataBuffer[x + 2] = '\0';
                break;
            } else
            {
                for (u16 i = 0; i < NulIndex; i++)
                {
                    CombinedDataBuffer[x + i + 1] = buffer[NulIndex - i - 1];
                }
            }

            CombinedDataBuffer[x + NulIndex + 1] = '\0';
            break;
        }
    }

}

u32
CountOfPerfectScore(struct id_details *GlobalIdDetails,
        struct id_data *GlobalIdData)
{
    u32 Counter = 0;
    for (u32 x = 0; x < GlobalIdData->uSize; x++)
    {
        Counter += (GlobalIdDetails[x].ubOccurence >= MAX_COLUMN_SCORE);
    }
    return Counter;
}

void
InitializeCombiner(struct combined *CombinedData,
        struct id_details *GlobalIdDetails, struct id_data *GlobalIdData)
{
    u32 CounterForCombiner = 0;
    for (u32 x = 0; x < GlobalIdData->uSize; x++)
    {
        if (GlobalIdDetails[x].ubOccurence >= MAX_COLUMN_SCORE)
        {
            CombinedData[CounterForCombiner].uID = GlobalIdDetails[x].uID;
            CounterForCombiner++;
        }
    }
}


void
GameCombine(struct combined *CombinedData, struct games *GameTable,
        u32 TotalRowsOfCombiner)
{
    u32 CounterForCombiner = 0;
    u32 StartIndex = 0;

    while (CombinedData[CounterForCombiner].uID > GameTable -> Data[0].uID)
    {
        StartIndex++;
    }
    
    for (u32 x = StartIndex; x < GameTable -> uSizeArray; x++)
    {
        while (GameTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;

            if (CounterForCombiner > TotalRowsOfCombiner)
            {
                break;
            }
        
        }

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].sName,
                sizeof(GameTable->Data[x].sName));

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].sReleaseDate,
                sizeof(GameTable->Data[x].sReleaseDate));

        ConvertIntToTheString(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].ubIsFree);

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].sType,
                sizeof(GameTable->Data[x].sType));
    }
}

void
CategoryCombine(combined *CombinedData, struct category *CategoryTable,
        u32 TotalRowsOfCombiner)
{
    u32 CounterForCombiner = 0;
    u32 StartIndex = 0;
    u32 CounterSwitch = 0;

    while (CombinedData[CounterForCombiner].uID > CategoryTable -> Data[0].uID)
    {
        StartIndex++;
    }

    for (u32 x = 0; x < CategoryTable -> uSizeArray; x++)
    {
        while (CategoryTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;
            CounterSwitch = 0;

            if (CounterForCombiner > TotalRowsOfCombiner)
            {
                break;
            }
        }

        if (CounterSwitch < 4)
        {
            CopyTheStringInCombiner(
                    CombinedData[CounterForCombiner].sData,
                    CategoryTable->Data[x].sCategory,
                    sizeof(CategoryTable->Data[x].sCategory));
            CounterSwitch++;

            if (CombinedData[CounterForCombiner].uID == 12600)
            {
                printf("%u\n", CombinedData[CounterForCombiner].uID);
                printf("%s\n", CombinedData[CounterForCombiner].sData);
            }
        }
    }
}

void
GenreCombine(combined *CombinedData, struct genre *GenreTable,
        u32 TotalRowsOfCombiner)
{
    u32 CounterForCombiner = 0;
    u32 StartIndex = 0;
    u32 CounterSwitch = 0;

    while (CombinedData[CounterForCombiner].uID > GenreTable -> Data[0].uID)
    {
        StartIndex++;
    }

    for (u32 x = 0; x < GenreTable -> uSizeArray; x++)
    {
        while (GenreTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;
            CounterSwitch = 0;

            if (CounterForCombiner > TotalRowsOfCombiner)
            {
                break;
            }
        }

        if (CounterSwitch < 4)
        {
            CopyTheStringInCombiner(
                    CombinedData[CounterForCombiner].sData,
                    GenreTable->Data[x].sGenre,
                    sizeof(GenreTable->Data[x].sGenre));
            CounterSwitch++;

            if (CombinedData[CounterForCombiner].uID == 12600)
            {
                printf("%u\n", CombinedData[CounterForCombiner].uID);
                printf("%s\n", CombinedData[CounterForCombiner].sData);
            }
        }
    }
}

void
SummaryCombine(combined *CombinedData, struct summary *SummaryTable,
        u32 TotalRowsOfCombiner)
{
    u32 CounterForCombiner = 0;
    u32 StartIndex = 0;
    u8 CounterSwitch = 0;

    while (CombinedData[CounterForCombiner].uID > SummaryTable -> Data[0].uID)
    {
        StartIndex++;
    }

    for (u32 x = 0; x < SummaryTable -> uSizeArray; x++)
    {
        while (SummaryTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;
            CounterSwitch = 0;

            if (CounterForCombiner > TotalRowsOfCombiner)
            {
                break;
            }
        }

        if (CounterSwitch < 1)
        {
        if (CombinedData[CounterForCombiner].uID == 12600)
        {
            printf("%u\n", CombinedData[CounterForCombiner].uID);
            printf("%s\n", CombinedData[CounterForCombiner].sData);
        }

            CopyTheStringInCombiner(
                    CombinedData[CounterForCombiner].sData,
                    SummaryTable->Data[x].sSummary,
                    sizeof(SummaryTable->Data[x].sSummary));

            CounterSwitch++;
        }
    }
}

void
OtherCombine(combined *CombinedData, struct other *OtherTable,
        u32 TotalRowsOfCombiner)
{
    u32 CounterForCombiner = 0;
    u32 StartIndex = 0;

    while (CombinedData[CounterForCombiner].uID > OtherTable -> Data[0].uID)
    {
        StartIndex++;
    }

    for (u32 x = 0; x < OtherTable -> uSizeArray; x++)
    {
        while (OtherTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;

            if (CounterForCombiner > TotalRowsOfCombiner)
            {
                break;
            }
        }

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                OtherTable->Data[x].sDeveloper,
                sizeof(OtherTable->Data[x].sDeveloper));

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                OtherTable->Data[x].sPublisher,
                sizeof(OtherTable->Data[x].sDeveloper));

        ConvertFloatIntoString(
                CombinedData[CounterForCombiner].sData,
                (u64) (OtherTable ->Data[x].fPrice * 100));

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                OtherTable->Data[x].sLanguage,
                sizeof(OtherTable->Data[x].sDeveloper));
    }
}


void
CombineIntoString(combined *CombinedData, struct games *GameTable,
        struct category *CategoryTable, struct genre *GenreTable,
        struct summary *SummaryTable,struct other *OtherTable,
        struct id_details *GlobalIdDetails, struct id_data *GlobalIdData,
        u32 TotalRowsOfCombiner)
{
    InitializeCombiner(CombinedData, GlobalIdDetails, GlobalIdData);
    GameCombine(CombinedData, GameTable, TotalRowsOfCombiner);
    CategoryCombine(CombinedData, CategoryTable, TotalRowsOfCombiner);
    GenreCombine(CombinedData, GenreTable, TotalRowsOfCombiner);
    SummaryCombine(CombinedData, SummaryTable, TotalRowsOfCombiner);
    OtherCombine(CombinedData, OtherTable, TotalRowsOfCombiner);
}


void
ThreadInitializerForCombiner(void * table, u8 TotalThreads,
        struct combined * CombinedData, u32 SizeArray,
        struct combiner_threads * Workers, u32 TotalRowsOfCombiner)
{
    u32 StepSize = SizeArray / TotalThreads;
    u8 Extra = SizeArray - (TotalThreads * StepSize);

    for (u32 x = 0; x < TotalThreads; x++)
    {
        Workers[x].table = table;
        Workers[x].CombinedData = CombinedData;
        Workers[x].uStepSize = StepSize;
        Workers[x].Extra = 0;
        Workers[x].TotalRowsOfCombiner = TotalRowsOfCombiner;
        Workers[x].ThreadIndex = x;
    }
    
    Workers[TotalThreads - 1].Extra = Extra;
}

DWORD WINAPI
GameCombineMultiThread(LPVOID lpParameter)
{
    struct combiner_threads * Workers = (struct combiner_threads *)
        lpParameter;

    struct combined * CombinedData = Workers -> CombinedData;
    struct games * GameTable = (struct games *) Workers -> table;
    u32 CounterForCombiner = 0;

    u32 StartIndex = Workers -> uStepSize * Workers -> ThreadIndex;
    u32 EndIndex = StartIndex + Workers -> uStepSize + Workers -> Extra;

    while (CombinedData[CounterForCombiner].uID >
            GameTable -> Data[StartIndex].uID)
    {
        StartIndex++;
    }

    while (CombinedData[CounterForCombiner].uID < GameTable->Data[StartIndex].uID)
    {
        if ((CounterForCombiner + 1024) > Workers->TotalRowsOfCombiner)
        {
            CounterForCombiner += (Workers->TotalRowsOfCombiner -
                    CounterForCombiner - 1);
            break;
        }

        CounterForCombiner += 1024;
    }
    //printf("%u\n", CounterForCombiner);

    for (u16 x = 0; x < 1023; x++) {
        if (CombinedData[CounterForCombiner].uID == GameTable->Data[StartIndex].uID) {
            break;
        }
        CounterForCombiner--;
    }
    //printf("%u\n", CounterForCombiner);
    //printf("StartIndex: %u\n", StartIndex);
    
    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        while (GameTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;

            if (CounterForCombiner > Workers -> TotalRowsOfCombiner)
            {
                break;
            }
        
        }

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].sName,
                sizeof(GameTable->Data[x].sName));

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].sReleaseDate,
                sizeof(GameTable->Data[x].sReleaseDate));

        ConvertIntToTheString(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].ubIsFree);

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                GameTable->Data[x].sType,
                sizeof(GameTable->Data[x].sType));
    }
    return 0;
}

DWORD WINAPI
CategoryCombineMultiThread(LPVOID lpParameter)
{
    struct combiner_threads * Workers = (struct combiner_threads *)
        lpParameter;

    struct combined * CombinedData = Workers -> CombinedData;
    struct category * CategoryTable = (struct category*) Workers -> table;

    u32 CounterForCombiner = 0;
    u8 CounterSwitch = 0;

    u32 StartIndex = Workers -> uStepSize * Workers -> ThreadIndex;
    u32 EndIndex = StartIndex + Workers -> uStepSize + Workers -> Extra;

    while (CombinedData[CounterForCombiner].uID >
            CategoryTable -> Data[StartIndex].uID)
    {
        StartIndex++;
    }

    while (CombinedData[CounterForCombiner].uID < CategoryTable->Data[StartIndex].uID)
    {
        if ((CounterForCombiner + 1024) > Workers->TotalRowsOfCombiner)
        {
            CounterForCombiner += (Workers->TotalRowsOfCombiner -
                    CounterForCombiner);
            break;
        }

        CounterForCombiner += 1024;
    }

    for (u16 x = 0; x < 1023; x++) {
        if (CombinedData[CounterForCombiner].uID == CategoryTable->Data[StartIndex].uID) {
            break;
        }
        CounterForCombiner--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        while (CategoryTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;
            CounterSwitch = 0;

            if (CounterForCombiner > Workers -> TotalRowsOfCombiner)
            {
                break;
            }
        }

        if (CounterSwitch < 4)
        {
            CopyTheStringInCombiner(
                    CombinedData[CounterForCombiner].sData,
                    CategoryTable->Data[x].sCategory,
                    sizeof(CategoryTable->Data[x].sCategory));
            CounterSwitch++;
        }
    }
    return 0;
}

DWORD WINAPI
GenreCombineMultiThread(LPVOID lpParameter)
{
    struct combiner_threads * Workers = (struct combiner_threads *)
        lpParameter;

    struct combined * CombinedData = Workers -> CombinedData;
    struct genre* GenreTable= (struct genre*) Workers -> table;

    u32 CounterForCombiner = 0;
    u8 CounterSwitch = 0;

    u32 StartIndex = Workers -> uStepSize * Workers -> ThreadIndex;
    u32 EndIndex = StartIndex + Workers -> uStepSize + Workers -> Extra;

    while (CombinedData[CounterForCombiner].uID > GenreTable -> Data[0].uID)
    {
        StartIndex++;
    }

    while (CombinedData[CounterForCombiner].uID <
            GenreTable->Data[StartIndex].uID)
    {
        if ((CounterForCombiner + 1024) > Workers->TotalRowsOfCombiner)
        {
            CounterForCombiner += (Workers->TotalRowsOfCombiner -
                    CounterForCombiner);
            break;
        }

        CounterForCombiner += 1024;
    }

    for (u16 x = 0; x < 1023; x++) {
        if (CombinedData[CounterForCombiner].uID == GenreTable->Data[StartIndex].uID) {
            break;
        }
        CounterForCombiner--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        while (GenreTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;
            CounterSwitch = 0;

            if (CounterForCombiner > Workers -> TotalRowsOfCombiner)
            {
                break;
            }
        }

        if (CounterSwitch < 4)
        {
            CopyTheStringInCombiner(
                    CombinedData[CounterForCombiner].sData,
                    GenreTable->Data[x].sGenre,
                    sizeof(GenreTable->Data[x].sGenre));
            CounterSwitch++;
        }
    }
    return 0;
}

DWORD WINAPI
SummaryCombineMultiThread(LPVOID lpParameter)
{
    struct combiner_threads * Workers = (struct combiner_threads *)
        lpParameter;

    struct combined * CombinedData = Workers -> CombinedData;
    struct summary* SummaryTable = (struct summary*) Workers -> table;

    u32 CounterForCombiner = 0;
    u8 CounterSwitch = 0;

    u32 StartIndex = Workers -> uStepSize * Workers -> ThreadIndex;
    u32 EndIndex = StartIndex + Workers -> uStepSize + Workers -> Extra;

    while (CombinedData[CounterForCombiner].uID >
            SummaryTable->Data[StartIndex].uID)
    {
        StartIndex++;
    }

    while (CombinedData[CounterForCombiner].uID < SummaryTable->Data[StartIndex].uID)
    {
        if ((CounterForCombiner + 1024) > Workers->TotalRowsOfCombiner)
        {
            CounterForCombiner += (Workers->TotalRowsOfCombiner -
                    CounterForCombiner);
            break;
        }

        CounterForCombiner += 1024;
    }

    for (u16 x = 0; x < 1023; x++) {
        if (CombinedData[CounterForCombiner].uID == SummaryTable->Data[StartIndex].uID) {
            break;
        }
        CounterForCombiner--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        while (SummaryTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;
            CounterSwitch = 0;

            if (CounterForCombiner > Workers -> TotalRowsOfCombiner)
            {
                break;
            }
        }

        if (CounterSwitch < 1)
        {

            CopyTheStringInCombiner(
                    CombinedData[CounterForCombiner].sData,
                    SummaryTable->Data[x].sSummary,
                    sizeof(SummaryTable->Data[x].sSummary));

            CounterSwitch++;
        }
    }
    return 0;
}

DWORD WINAPI
OtherCombineMultiThread(LPVOID lpParameter)
{
    struct combiner_threads * Workers = (struct combiner_threads *)
        lpParameter;

    struct combined * CombinedData = Workers -> CombinedData;
    struct other * OtherTable = (struct other*) Workers -> table;

    u32 CounterForCombiner = 0;
    u8 CounterSwitch = 0;

    u32 StartIndex = Workers -> uStepSize * Workers -> ThreadIndex;
    u32 EndIndex = StartIndex + Workers -> uStepSize + Workers -> Extra;

    while (CombinedData[CounterForCombiner].uID >
            OtherTable -> Data[StartIndex].uID)
    {
        StartIndex++;
    }

    while (CombinedData[CounterForCombiner].uID <
            OtherTable->Data[StartIndex].uID)
    {
        if ((CounterForCombiner + 1024) > Workers->TotalRowsOfCombiner)
        {
            CounterForCombiner += (Workers->TotalRowsOfCombiner -
                    CounterForCombiner);
            break;
        }

        CounterForCombiner += 1024;
    }

    for (u16 x = 0; x < 1023; x++) {
        if (CombinedData[CounterForCombiner].uID == OtherTable->Data[StartIndex].uID) {
            break;
        }
        CounterForCombiner--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        while (OtherTable -> Data[x].uID >
                CombinedData[CounterForCombiner].uID)
        {
            CounterForCombiner++;

            if (CounterForCombiner > Workers -> TotalRowsOfCombiner)
            {
                break;
            }
        }

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                OtherTable->Data[x].sDeveloper,
                sizeof(OtherTable->Data[x].sDeveloper));

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                OtherTable->Data[x].sPublisher,
                sizeof(OtherTable->Data[x].sDeveloper));

        ConvertFloatIntoString(
                CombinedData[CounterForCombiner].sData,
                (u64) (OtherTable ->Data[x].fPrice * 100));

        CopyTheStringInCombiner(
                CombinedData[CounterForCombiner].sData,
                OtherTable->Data[x].sLanguage,
                sizeof(OtherTable->Data[x].sDeveloper));
    }
    return 0;
}
