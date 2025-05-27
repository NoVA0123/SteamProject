/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include "typedef.h"
#include "csv_parser.h"
#include "data_cleaning.h"
#include <windows.h>
#include <processthreadsapi.h>
#include <stdio.h>


void
InitializeGlobalIdDetails(struct games * GameTable,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData)
{
    for (u32 x = 0; x < GameTable -> uSizeArray; x++)
    {
        GlobalIdDetails[x].uID = GameTable -> Data[x].uID;
        GlobalIdDetails[x].ubOccurence = 0;
    }

    GlobalIdData -> uSize = GameTable -> uSizeArray;
}


void
InitializeMultiThreadFunctionParameter(void * table, u8 TotalThreads,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData,
        u32 SizeArray, struct function_parameter_data * Nexus)
{
    u32 StepSize = SizeArray / TotalThreads;
    u8 Extra = SizeArray - (TotalThreads * StepSize);

    for (u32 x = 0; x < TotalThreads; x++)
    {
        Nexus[x].table = table;
        Nexus[x].GlobalIdDetails = GlobalIdDetails;
        Nexus[x].GlobalIdData = GlobalIdData;
        Nexus[x].uStepSize = StepSize;
        Nexus[x].ThreadIndex = x;
        Nexus[x].Extra = 0;
    }

    Nexus[TotalThreads - 1].Extra = Extra;
}


u8
CheckStringAnomaly(char * buffer, u16 Size)
{
    u8 NotDetectedAnomaly = 1;

    if (buffer[0] == '\0')
    {
        NotDetectedAnomaly = 0;
    }

    if (buffer[0] == '\\')
    {
        if (buffer[1] == 'N')
        {
            NotDetectedAnomaly = 0;
        }
    }

    return NotDetectedAnomaly;
}


void
CheckGamesTable(struct games * GameTable, struct id_details * GlobalIdDetails)
{
    for (u32 x = 0; x < GameTable -> uSizeArray; x++)
    {
        GlobalIdDetails[x].ubOccurence += 
            CheckStringAnomaly(GameTable -> Data[x].sName,
                    sizeof(GameTable -> Data[x].sName));

        GlobalIdDetails[x].ubOccurence +=
            CheckStringAnomaly(GameTable -> Data[x].sReleaseDate,
                    sizeof(GameTable -> Data[x].sReleaseDate));

        GlobalIdDetails[x].ubOccurence +=
            CheckStringAnomaly(GameTable -> Data[x].sType,
                    sizeof(GameTable -> Data[x].sType));

        GlobalIdDetails[x].ubOccurence += 2;
    }
}

DWORD WINAPI
CheckGamesTableMultiThread(LPVOID lpParameter)
{
    function_parameter_data * Nexus = (function_parameter_data *) lpParameter;
    games * GameTable = (games *) Nexus -> table;
    id_details * GlobalIdDetails = Nexus -> GlobalIdDetails;
    
    u32 StartIndex = Nexus -> uStepSize * Nexus -> ThreadIndex;
    u32 EndIndex = StartIndex + Nexus -> uStepSize + Nexus -> Extra;
    
    if (EndIndex > GameTable->uSizeArray)
    {
        printf("Access violation in GameTable\n");
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        GlobalIdDetails[x].ubOccurence +=
            CheckStringAnomaly(GameTable -> Data[x].sName,
                    sizeof(GameTable -> Data[x].sName));

        GlobalIdDetails[x].ubOccurence +=
            CheckStringAnomaly(GameTable -> Data[x].sReleaseDate,
                    sizeof(GameTable -> Data[x].sReleaseDate));

        GlobalIdDetails[x].ubOccurence +=
            CheckStringAnomaly(GameTable -> Data[x].sType,
                    sizeof(GameTable -> Data[x].sType));

        GlobalIdDetails[x].ubOccurence += 2;
    }

    return 0;
}

void
CheckCategoryTable(struct category * CategoryTable,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData)
{
    u32 SizeArray = CategoryTable -> uSizeArray;
    u32 Counter = 0;
    u8 CounterSwitch = 0;

    for (u32 x = 0; x < SizeArray; x++)
    {
        if (Counter > GlobalIdData -> uSize)
        {
            break;
        }

        while (GlobalIdDetails[Counter].uID != CategoryTable -> Data[x].uID)
        {
            Counter++;
            CounterSwitch = 0;
        }

        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 1;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(CategoryTable -> Data[x].sCategory,
                    sizeof(CategoryTable -> Data[x].sCategory));

            CounterSwitch = 1;
        }
    }
}

DWORD WINAPI
CheckCategoryTableMultiThread(LPVOID lpParameter)
{
    function_parameter_data * Nexus = (function_parameter_data *) lpParameter;

    category * CategoryTable = (category *) Nexus -> table;
    id_details * GlobalIdDetails = Nexus -> GlobalIdDetails;
    id_data * GlobalIdData = Nexus -> GlobalIdData;

    u32 StartIndex = ((Nexus -> uStepSize * Nexus -> ThreadIndex));
    u32 EndIndex = StartIndex + Nexus -> uStepSize + Nexus -> Extra;
    u32 CurrentID = 0;

    if (EndIndex > CategoryTable->uSizeArray)
    {
        printf("Access violation in CategoryTable\n");
    }

    if (StartIndex)
    {
        if (CategoryTable->Data[StartIndex].uID == CategoryTable->Data[StartIndex - 1].uID)
        {
            CurrentID = CategoryTable->Data[StartIndex].uID;

            while (CurrentID == CategoryTable->Data[StartIndex].uID)
            {
                StartIndex++;
            }
        }
    }

    if ((EndIndex + 1) < CategoryTable->uSizeArray)
    {
        CurrentID =  CategoryTable->Data[EndIndex].uID;

        if (CurrentID == CategoryTable->Data[EndIndex + 1].uID)
        {
           while (CurrentID == CategoryTable->Data[EndIndex].uID)
           {
               EndIndex++;
           } 

           EndIndex--;
        }
    }

    u32 Counter = 0;
    u8 CounterSwitch = 0;

    while (GlobalIdDetails[Counter].uID < CategoryTable->Data[StartIndex].uID)
    {
        Counter += 1024;
    }

    for (u16 x = 0; x < 1023; x++)
    {
        if (GlobalIdDetails[Counter].uID ==
                CategoryTable->Data[StartIndex].uID)
        {
            break;
        }

        Counter--;
    }

    //printf("ThreadIndex: %u %u\n", Nexus ->ThreadIndex, StartIndex);
    //printf("ThreadIndex: %u %u\n", Nexus ->ThreadIndex, EndIndex);
    //printf("Counter value: %u\n", Counter);

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        if (Counter > GlobalIdData -> uSize)
        {
            break;
        }

        while (GlobalIdDetails[Counter].uID != CategoryTable -> Data[x].uID)
        {
            Counter++;
            //printf("Counter value: %u\n", Counter);

            if (Counter > GlobalIdData -> uSize)
            {
                break;
            }

            CounterSwitch = 0;
        }
        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 1;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(CategoryTable -> Data[x].sCategory,
                    sizeof(CategoryTable -> Data[x].sCategory));

            CounterSwitch = 1;
        }
    }
    return 0;
}

void CheckGenreTable(struct genre * GenreTable,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData)
{
    u32 SizeArray = GenreTable -> uSizeArray;
    u32 Counter = 0;
    u8 CounterSwitch = 0;

    for (u32 x = 0; x < SizeArray; x++)
    {
        while (GlobalIdDetails[Counter].uID != GenreTable -> Data[x].uID)
        {
            Counter++;
            CounterSwitch = 0;
        }

        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 1;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(GenreTable -> Data[x].sGenre,
                    sizeof(GenreTable -> Data[x].sGenre));

            CounterSwitch = 1;
        }
    }
}

DWORD WINAPI
CheckGenreTableMultiThread(LPVOID lpParameter)
{
    function_parameter_data * Nexus = (function_parameter_data *) lpParameter;

    genre * GenreTable = (genre *) Nexus -> table;
    id_details * GlobalIdDetails = Nexus -> GlobalIdDetails;
    id_data * GlobalIdData = Nexus -> GlobalIdData;

    u32 StartIndex = ((Nexus -> uStepSize * Nexus -> ThreadIndex));
    u32 EndIndex = StartIndex + Nexus -> uStepSize + Nexus -> Extra;

    if (EndIndex > GenreTable->uSizeArray)
    {
        printf("Access violation in GenreTable\n");
    }

    u32 Counter = 0;
    u8 CounterSwitch = 0;

    while (GlobalIdDetails[Counter].uID < GenreTable->Data[StartIndex].uID)
    {
        Counter += 1024;
    }

    for (u16 x = 0; x < 1023; x++)
    {
        if (GlobalIdDetails[Counter].uID == GenreTable->Data[StartIndex].uID)
        {
            break;
        }

        Counter--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        if (Counter > GlobalIdData -> uSize)
        {
            break;
        }

        while (GlobalIdDetails[Counter].uID != GenreTable -> Data[x].uID)
        {
            Counter++;

            if (Counter > GlobalIdData -> uSize)
            {
                break;
            }

            CounterSwitch = 0;
        }

        if (CounterSwitch < 1)
        {
            GlobalIdDetails[Counter].ubOccurence += 1;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(GenreTable -> Data[x].sGenre,
                    sizeof(GenreTable -> Data[x].sGenre));

            CounterSwitch++;
        }
    }
    return 0;
}

void
CheckSummaryTable(struct summary * SummaryTable,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData)
{
    u32 SizeArray = SummaryTable -> uSizeArray;
    u32 Counter = 0;
    u8 CounterSwitch = 0;

    for (u32 x = 0; x < SizeArray; x++)
    {
        while (GlobalIdDetails[Counter].uID != SummaryTable -> Data[x].uID)
        {
            Counter++;
            CounterSwitch = 0;
        }

        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 1;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(SummaryTable -> Data[x].sSummary,
                    sizeof(SummaryTable -> Data[x].sSummary));

            CounterSwitch = 1;
        }
    }
}

DWORD WINAPI
CheckSummaryTableMultiThread(LPVOID lpParameter)
{
    function_parameter_data * Nexus = (function_parameter_data *) lpParameter;

    summary * SummaryTable = (summary *) Nexus -> table;
    id_details * GlobalIdDetails = Nexus -> GlobalIdDetails;
    id_data * GlobalIdData = Nexus -> GlobalIdData;

    u32 StartIndex = ((Nexus -> uStepSize * Nexus -> ThreadIndex));
    u32 EndIndex = StartIndex + Nexus -> uStepSize + Nexus -> Extra;

    if (EndIndex > SummaryTable->uSizeArray)
    {
        printf("Access violation in SummaryTable\n");
    }

    u32 Counter = 0;
    u8 CounterSwitch = 0;

    while (GlobalIdDetails[Counter].uID < SummaryTable->Data[StartIndex].uID)
    {
        Counter += 1024;
    }

    for (u16 x = 0; x < 1023; x++)
    {
        if (GlobalIdDetails[Counter].uID == SummaryTable->Data[StartIndex].uID)
        {
            break;
        }

        Counter--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        if (Counter > GlobalIdData -> uSize)
        {
            break;
        }

        while (GlobalIdDetails[Counter].uID != SummaryTable -> Data[x].uID)
        {
            Counter++;

            if (Counter > GlobalIdData -> uSize)
            {
                break;
            }

            CounterSwitch = 0;
        }
        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 1;

            GlobalIdDetails[Counter].ubOccurence += 
                CheckStringAnomaly(SummaryTable -> Data[x].sSummary,
                    sizeof(SummaryTable -> Data[x].sSummary));

            CounterSwitch = 1;
        }
    }
    return 0;
}

void
CheckOtherTable(struct other * OtherTable,
        struct id_details * GlobalIdDetails, struct id_data * GlobalIdData)
{
    u32 SizeArray = OtherTable -> uSizeArray;
    u32 Counter = 0;
    u8 CounterSwitch = 0;

    for (u32 x = 0; x < SizeArray; x++)
    {
        if (Counter > GlobalIdData -> uSize)
        {
            break;
        }

        while (GlobalIdDetails[Counter].uID != OtherTable -> Data[x].uID)
        {
            Counter++;
            CounterSwitch = 0;
        }

        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 2;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(OtherTable -> Data[x].sDeveloper,
                    sizeof(OtherTable -> Data[x].sDeveloper));

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(OtherTable -> Data[x].sPublisher,
                    sizeof(OtherTable -> Data[x].sPublisher));

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(OtherTable -> Data[x].sLanguage,
                    sizeof(OtherTable -> Data[x].sLanguage));

            CounterSwitch = 1;
        }
    }
}

DWORD WINAPI
CheckOtherTableMultiThread(LPVOID lpParameter
        )
{
    function_parameter_data * Nexus = (function_parameter_data *) lpParameter;

    other * OtherTable = (other *) Nexus -> table;
    id_details * GlobalIdDetails = Nexus -> GlobalIdDetails;
    id_data * GlobalIdData = Nexus -> GlobalIdData;

    u32 StartIndex = ((Nexus -> uStepSize * Nexus -> ThreadIndex));
    u32 EndIndex = StartIndex + Nexus -> uStepSize + Nexus -> Extra;

    if (EndIndex > OtherTable->uSizeArray)
    {
        printf("Access violation in OtherTable\n");
    }

    u32 Counter = 0;
    u8 CounterSwitch = 0;

    while (GlobalIdDetails[Counter].uID < OtherTable->Data[StartIndex].uID)
    {
        Counter += 1024;
    }

    for (u16 x = 0; x < 1023; x++)
    {
        if (GlobalIdDetails[Counter].uID == OtherTable->Data[StartIndex].uID)
        {
            break;
        }
        Counter--;
    }

    for (u32 x = StartIndex; x < EndIndex; x++)
    {
        if (Counter > GlobalIdData -> uSize)
        {
            break;
        }

        while (GlobalIdDetails[Counter].uID != OtherTable -> Data[x].uID)
        {
            Counter++;

            if (Counter > GlobalIdData -> uSize)
            {
                break;
            }

            CounterSwitch = 0;
        }

        if (!CounterSwitch)
        {
            GlobalIdDetails[Counter].ubOccurence += 2;

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(OtherTable -> Data[x].sDeveloper,
                    sizeof(OtherTable -> Data[x].sDeveloper));

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(OtherTable -> Data[x].sPublisher,
                    sizeof(OtherTable -> Data[x].sPublisher));

            GlobalIdDetails[Counter].ubOccurence +=
                CheckStringAnomaly(OtherTable -> Data[x].sLanguage,
                    sizeof(OtherTable -> Data[x].sLanguage));

            CounterSwitch = 1;
        }
    }
    return 0;
}


void
CompareAllData(struct games * GameTable, struct category * CategoryTable,
        struct genre * GenreTable, struct summary * SummaryTable,
        struct other * OtherTable, struct id_details * GlobalIdDetails,
        struct id_data * GlobalIdData)
{
    InitializeGlobalIdDetails(GameTable, GlobalIdDetails, GlobalIdData);

    CheckGamesTable(GameTable, GlobalIdDetails);
    CheckCategoryTable(CategoryTable, GlobalIdDetails, GlobalIdData);
    CheckGenreTable(GenreTable, GlobalIdDetails, GlobalIdData);
    CheckSummaryTable(SummaryTable, GlobalIdDetails, GlobalIdData);
    CheckOtherTable(OtherTable, GlobalIdDetails, GlobalIdData);
}
