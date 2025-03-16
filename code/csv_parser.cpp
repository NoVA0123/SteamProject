/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <sys/stat.h>
#include <math.h>
#include <windows.h>
#include <winbase.h>
#include <processthreadsapi.h>
#include "typedef.h"
#include "csv_parser.h"

struct stat FileSize(char *pfileName) {
    struct stat Stat;
    stat(pfileName, &Stat);
    return Stat;
}

struct csv_info * GPTFindValuesFromBuffer(
        char * buffer,
        u32 * Values,
        u64 size,
        csv_info * GameInfo) {
    u32 Counter = 0;
    u8 CsvEscape = 0;
    u8 LineFeed = 0;
    for (u32 i = 0; i < size; i++) {
        if (buffer[i] == '"') {  /* 92 = \ */
            if (i == 0 || buffer[i-1] != '\\') {
                CsvEscape = !CsvEscape;
            }
        }
        if (!CsvEscape && (buffer[i] == ',' || buffer[i] == '\n')) {
            if (buffer[i] == '\n') {
                LineFeed = Counter;
            }
            Counter++;
        }
    }

    Counter = 0;
    for (u32 i = 0; i < size; i++) {
        if (buffer[i] == '"') {  /* 92 = \ */
            if (i == 0 || buffer[i-1] != '\\') {
                CsvEscape = !CsvEscape;
            }
        }
        if (!CsvEscape && (buffer[i] == ',' || buffer[i] == '\n')) {
            Values[Counter] = i;
            Counter++;
        }
    }
    GameInfo -> uSizeArray = Counter;
    GameInfo -> ubFirstLineFeed = LineFeed;
    return GameInfo;
}

struct csv_info * FindValuesFromBuffer(
        char * buffer,
        u32 * Values,
        u64 size,
        csv_info * GameInfo) {
    u32 Counter = 0;
    u8 CsvEscape = 0;
    u8 LineFeed = 0;
    for (u32 i = 0; i < size; i++) {
        if (buffer[i] == 92) {  /* 92 = \ */
            i++;
            continue;
        }
        CsvEscape = CsvEscape ^ (buffer[i] == '"');
        if (CsvEscape) {
            continue;
        }
        if ((!(buffer[i] - ',')) | (!(buffer[i] - '\n'))) {
            Values[Counter] = i;
            Counter += 1;
            if (!(buffer[i] - '\n')) {
                LineFeed = Counter;
                break;
            }
        }
    }
    Counter = 0;
    u8 TmpBool;
    for (u32 i = 0; i < size; i++) {
        /*92 = \\*/
        if (buffer[i] == 92) {
            i++;
            continue;
        }
        //i += 1 << (buffer[i] == 92);
        CsvEscape = CsvEscape ^ (buffer[i] == '"');
        //i += CsvEscape;
        if (CsvEscape) {
            continue;
        }
        TmpBool = !CsvEscape & (buffer[i] == ',' | buffer[i] == '\n');
        if (TmpBool) {
            Values[Counter] = i;
            Counter += 1;
        }
    }
    GameInfo -> uSizeArray = Counter;
    GameInfo -> ubFirstLineFeed = LineFeed;
    return GameInfo;
}

void CopyString(
        u32 location1,
        u32 location2,
        char * buffer,
        u16 max,
        char * values) {
    if (location1 >= location2) {
        values[0] = '"';
        values[1] = '"';
        values[2] = '\0';
        return;
    }
    u32 length = location2 - location1;
    length = length*(max > length) + max*(max <= length);
    for (u32 i=0; i < length; i++) {
        values[i] = *buffer++;
    }
    values[length] = '\0';
}

static inline u32 FastAtoi(const char * str) {
    u32 val = 0;
    if (*str >= '9') {
        return 0;
    }
    while (*str > '"') {
        val = (val * 10) + (*str++ - '0');
    }
    return val;
}

void GamesParseSingleThread(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    u32 Counter = 0;
    char * location1 = 0;
    char * location2 = 0;
    for (u32 x = 6; x < SizeArray; x += 7) {
        // location1 = &buffer[location[x] + 2];
        location1 = buffer + (location[x] + 2);
        table->Data[Counter].uID = FastAtoi(location1);
        CopyString(
                location[x+1] + 2,
                location[x+2] - 1,
                // &buffer[location[x+1] + 2],
                buffer + (location[x+1] + 2),
                sizeof(table->Data[Counter].sName),
                table->Data[Counter].sName);
        CopyString(
                location[x+2] + 2,
                location[x+3] - 1,
                // &buffer[location[x+2] + 2],
                buffer + (location[x+2] + 2),
                sizeof(table->Data[Counter].sReleaseDate),
                table->Data[Counter].sReleaseDate);
        // location2 = &buffer[location[x] + 3];
        location2 = buffer + (location[x] + 3);
        table->Data[Counter].ubIsFree = (u8) FastAtoi(location2);
        CopyString(
                location[x+6] + 2,
                location[x+7] - 1,
                // &buffer[location[x+2] + 2],
                buffer + (location[x+2] + 6),
                sizeof(table->Data[Counter].sType),
                table->Data[Counter].sType);

        Counter++;
    }
}

void CategoryParseSingleThread(
        struct category *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    char * location1 = 0;
    u32 Counter = 0;
    for (u32 x = 2; x < SizeArray; x += 3) {
        location1 = buffer + (location[x] + 2);
        table->Data[Counter].uID = FastAtoi(location1);
        CopyString(
                location[x+1] + 2,
                location[x+2] - 1,
                // &buffer[location[x+1] + 2],
                buffer + (location[x+1] + 2),
                sizeof(table->Data[Counter].sCategory),
                table->Data[Counter].sCategory);
        Counter++;
    }
}

void ThreadInitializer(
        void * table,
        char * buffer,
        u32 * locations,
        u8 TotalThreads,
        thread_info * Info,
        csv_info * GameInfo
        ) {
    u32 SizeArray = GameInfo -> uSizeArray;
    u32 BaseSize = (SizeArray - GameInfo -> ubFirstLineFeed) /
        GameInfo -> ubFirstLineFeed;
    u32 StepSize = BaseSize / TotalThreads;
    u32 Extra = BaseSize - (StepSize * TotalThreads);

    for (u8 x = 0; x < TotalThreads; x++) {
        Info[x].table = table;
        Info[x].uSizeArray = SizeArray;
        Info[x].caBuffer = buffer;
        Info[x].uaLocation = locations;
        Info[x].uStepSize = StepSize;
        Info[x].ThreadIndex = x;
        Info[x].ubFirstLineFeed = GameInfo -> ubFirstLineFeed;
        Info[x].Extra = 0;
    }
    Info[TotalThreads - 1].Extra = Extra;
}

DWORD WINAPI GamesParseMultiThread(
        LPVOID lpParameter) {
    thread_info * Info = (thread_info *) lpParameter;
    games * table = (games *) Info -> table;

    u32 StartIndex = ((Info -> uStepSize * Info -> ThreadIndex) *
        Info -> ubFirstLineFeed) + (Info -> ubFirstLineFeed - 1);
    u32 EndIndex = StartIndex + ((Info -> uStepSize + Info -> Extra) *
            Info -> ubFirstLineFeed);
    u32 Counter = (Info -> uStepSize * Info -> ThreadIndex);

    char * location1 = 0;
    char * location2 = 0;

    for (u32 x = StartIndex; x < EndIndex; x += Info -> ubFirstLineFeed) {
        // location1 = &(Info -> caBuffer[Info -> uaLocation[x] + 2]);
        location1 = Info -> caBuffer + (Info -> uaLocation[x] + 2);
        table -> Data[Counter].uID = FastAtoi(location1);

        CopyString(
                Info -> uaLocation[x+1] + 2,
                Info -> uaLocation[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+1] + 2),
                sizeof(table -> Data[Counter].sName),
                table -> Data[Counter].sName);

        CopyString(
                Info -> uaLocation[x+2] + 2,
                Info -> uaLocation[x+3] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+2] + 2),
                sizeof(table -> Data[Counter].sReleaseDate),
                table -> Data[Counter].sReleaseDate);

        // location2 = &(Info -> caBuffer[Info -> uaLocation[x] + 3]);
        location2 = Info -> caBuffer + (Info -> uaLocation[x+3]) + 2;
        table -> Data[Counter].ubIsFree = (u8) FastAtoi(location2);

        CopyString(
                Info -> uaLocation[x+6] + 2,
                Info -> uaLocation[x+7] - 1,
                Info -> caBuffer + (Info -> uaLocation[x+6] + 2),
                sizeof(table ->Data[Counter].sType),
                table -> Data[Counter].sType);

        Counter++;
    }
    if (Info -> ThreadIndex == 3) {
        table -> Data[Counter] = {};
    }
    return 0;
}

DWORD WINAPI CategoryParseMultiThread(
        LPVOID lpParameter) {
    thread_info * Info = (thread_info *) lpParameter;
    category * table = (category *) Info -> table;

    u32 StartIndex = ((Info -> uStepSize * Info -> ThreadIndex) *
        Info -> ubFirstLineFeed) + (Info -> ubFirstLineFeed - 1);
    u32 EndIndex = StartIndex + ((Info -> uStepSize + Info -> Extra) *
            Info -> ubFirstLineFeed);
    u32 Counter = (Info -> uStepSize * Info -> ThreadIndex);

    char * location1 = 0;
    char * location2 = 0;

    for (u32 x = StartIndex; x < EndIndex; x += Info -> ubFirstLineFeed) {
        // location1 = &(Info -> caBuffer[Info -> uaLocation[x] + 2]);
        location1 = Info -> caBuffer + (Info -> uaLocation[x] + 2);
        table -> Data[Counter].uID = FastAtoi(location1);

        CopyString(
                Info -> uaLocation[x+1] + 2,
                Info -> uaLocation[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+1] + 2),
                sizeof(table -> Data[Counter].sCategory),
                table -> Data[Counter].sCategory);
        Counter++;
    }
    return 0;
}

DWORD WINAPI SummaryParseMultiThread(
        LPVOID lpParameter) {
    thread_info * Info = (thread_info *) lpParameter;
    summary * table = (summary *) Info -> table;

    u32 StartIndex = ((Info -> uStepSize * Info -> ThreadIndex) *
        Info -> ubFirstLineFeed) + (Info -> ubFirstLineFeed - 1);
    u32 EndIndex = StartIndex + ((Info -> uStepSize + Info -> Extra) *
            Info -> ubFirstLineFeed);
    u32 Counter = (Info -> uStepSize * Info -> ThreadIndex);

    char * location1 = 0;

    for (u32 x = StartIndex; x < EndIndex; x += Info -> ubFirstLineFeed) {
        // location1 = &(Info -> caBuffer[Info -> uaLocation[x] + 2]);
        location1 = Info -> caBuffer + (Info -> uaLocation[x] + 2);
        table -> Data[Counter].uID = FastAtoi(location1);

        CopyString(
                Info -> uaLocation[x+1] + 2,
                Info -> uaLocation[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+1] + 2),
                sizeof(table -> Data[Counter].sSummary),
                table -> Data[Counter].sSummary);
        Counter++;
    }
    return 0;
}

DWORD WINAPI GenreParseMultiThread(
        LPVOID lpParameter) {
    thread_info * Info = (thread_info *) lpParameter;
    genre * table = (genre *) Info -> table;

    u32 StartIndex = ((Info -> uStepSize * Info -> ThreadIndex) *
        Info -> ubFirstLineFeed) + (Info -> ubFirstLineFeed - 1);
    u32 EndIndex = StartIndex + ((Info -> uStepSize + Info -> Extra) *
            Info -> ubFirstLineFeed);
    u32 Counter = (Info -> uStepSize * Info -> ThreadIndex);

    char * location1 = 0;

    for (u32 x = StartIndex; x < EndIndex; x += Info -> ubFirstLineFeed) {
        // location1 = &(Info -> caBuffer[Info -> uaLocation[x] + 2]);
        location1 = Info -> caBuffer + (Info -> uaLocation[x] + 2);
        table -> Data[Counter].uID = FastAtoi(location1);

        CopyString(
                Info -> uaLocation[x+1] + 2,
                Info -> uaLocation[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+1] + 2),
                sizeof(table -> Data[Counter].sGenre),
                table -> Data[Counter].sGenre);
        Counter++;
    }
    return 0;
}

DWORD WINAPI OtherParseMultiThread(
        LPVOID lpParameter) {
    thread_info * Info = (thread_info *) lpParameter;
    other * table = (other *) Info -> table;

    u32 StartIndex = ((Info -> uStepSize * Info -> ThreadIndex) *
        Info -> ubFirstLineFeed) + (Info -> ubFirstLineFeed - 1);
    u32 EndIndex = StartIndex + ((Info -> uStepSize + Info -> Extra) *
            Info -> ubFirstLineFeed);
    u32 Counter = (Info -> uStepSize * Info -> ThreadIndex);

    char * location1 = 0;
    char * location2 = 0;

    for (u32 x = StartIndex; x < EndIndex; x += Info -> ubFirstLineFeed) {
        // location1 = &(Info -> caBuffer[Info -> uaLocation[x] + 2]);
        location1 = Info -> caBuffer + (Info -> uaLocation[x] + 2);
        table -> Data[Counter].uID = FastAtoi(location1);

        CopyString(
                Info -> uaLocation[x+1] + 2,
                Info -> uaLocation[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+1] + 2),
                sizeof(table -> Data[Counter].sDeveloper),
                table -> Data[Counter].sDeveloper);

        CopyString(
                Info -> uaLocation[x+2] + 2,
                Info -> uaLocation[x+3] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+2] + 2),
                sizeof(table -> Data[Counter].sPublisher),
                table -> Data[Counter].sPublisher);

        location2 = Info -> caBuffer + (Info -> uaLocation[x + 9] + 2);
        table -> Data[Counter].fPrice = (f64)FastAtoi(location2) / (f64) 100;

        CopyString(
                Info -> uaLocation[x+12] + 2,
                Info -> uaLocation[x+13] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                Info -> caBuffer + (Info -> uaLocation[x+12] + 2),
                sizeof(table -> Data[Counter].sLanguage),
                table -> Data[Counter].sLanguage);
        Counter++;
    }
    return 0;
}

void GameColumnParse(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    for (u32 x = 0; x < 1; x++) {
        CopyString(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);

        CopyString(
                location[x] + 2,
                location[x+1] - 1,
                &buffer[location[x] + 2],
                sizeof(table->Column[1]),
                table->Column[1]);

        CopyString(
                location[x+1] + 2,
                location[x+2] - 1,
                &buffer[location[x+1] + 2],
                sizeof(table->Column[2]),
                table->Column[2]);

        CopyString(
                location[x+2] + 2,
                location[x+3] - 1,
                &buffer[location[x+2] + 2],
                sizeof(table->Column[3]),
                table->Column[3]);

        CopyString(
                location[x+5] + 2,
                location[x+6] - 1,
                &buffer[location[x+5] + 2],
                sizeof(table->Column[4]),
                table->Column[4]);
    }
}

void CategoryColumnParse(
        struct category *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    for (u32 x = 0; x < 1; x++) {
        CopyString(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);

        CopyString(
                location[x] + 2,
                location[x+1] - 1,
                &buffer[location[x] + 2],
                sizeof(table->Column[1]),
                table->Column[1]);
    }
}

void GenreColumnParse(
        struct genre *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    for (u32 x = 0; x < 1; x++) {
        CopyString(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);

        CopyString(
                location[x+1] + 2,
                location[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                buffer + (location[x+1] + 2),
                sizeof(table -> Column[1]),
                table -> Column[1]);
    }
}

void SummaryColumnParse(
        struct summary *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    for (u32 x = 0; x < 1; x++) {
        CopyString(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);

        CopyString(
                location[x+1] + 2,
                location[x+2] - 1,
                // &(Info -> caBuffer[Info -> uaLocation[x+2] + 2]),
                buffer + (location[x+1] + 2),
                sizeof(table -> Column[1]),
                table -> Column[1]);
    }
}

void OtherColumnParse(
        struct other *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    for (u32 x = 0; x < 1; x++) {
        CopyString(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);

        CopyString(
                location[x+1] + 2,
                location[x+2] - 1,
                &buffer[location[x+1] + 2],
                sizeof(table->Column[1]),
                table->Column[1]);
        CopyString(
                location[x+2] + 2,
                location[x+3] - 1,
                &buffer[location[x+2] + 2],
                sizeof(table->Column[2]),
                table->Column[2]);
        CopyString(
                location[x+12] + 2,
                location[x+13] - 1,
                &buffer[location[x+12] + 2],
                sizeof(table->Column[3]),
                table->Column[3]);
    }
}
