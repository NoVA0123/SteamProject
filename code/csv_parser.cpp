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
#include "platform_metrics.cpp"
#include "nested_profiler.cpp"
#include "typedef.h"

struct stat file_size(char *pfileName) {
    struct stat Stat;
    stat(pfileName, &Stat);
    return Stat;
}

struct thread_info {
    struct games *table;
    u32 SizeArray;
    char * buffer;
    u32 * location;
    u32 WorkStartIndex;
    u32 division_size;
    u8 extra;
};

struct GameColumns{
    u32 id;
    char name[32];
    char release_date[16];
    u8 is_free;
    char type[8];
};

struct games{
    struct GameColumns  * Data;
    char Column[5][13];
    // [TotalCols][10]
};

u32 find_values_from_buffer(
        char * buffer,
        u32 * Values,
        u64 size) {
    u32 Counter = 0;
    u8 CsvEscape = 0;
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
        }
    }
    return Counter;
}

static inline void load_string(
        u32 location1,
        u32 location2,
        char * buffer,
        u8 max,
        char * values) {
    if (location1 >= location2) {
        values[0] = '"';
        values[1] = '"';
        values[2] = '\0';
        return;
    }
    u16 length = location2 - location1;
    length = length*(max > length) + max*(max <= length);
    for (u8 i=0; i < length; i++) {
        values[i] = *buffer++;
    }
    values[length] = '\0';
}

static inline u32 fast_atoi(const char * str) {
    u32 val = 0;
    if (*str >= 'N') {
        return 0;
    }
    while (*str > '"') {
        val = (val * 10) + (*str++ - '0');
    }
    return val;
}

void game_parse(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    u32 Counter = 0;
    char * location1 = 0;
    char * location2 = 0;
    for (u32 x = 6; x < SizeArray; x += 7) {
        location1 = &buffer[location[x] + 2];
        table->Data[Counter].id = fast_atoi(location1);
        load_string(
                location[x+1] + 2,
                location[x+2] - 1,
                &buffer[location[x+1] + 2],
                sizeof(table->Data[Counter].name),
                table->Data[Counter].name);
        load_string(
                location[x+2] + 2,
                location[x+3] - 1,
                &buffer[location[x+2] + 2],
                sizeof(table->Data[Counter].release_date),
                table->Data[Counter].release_date);
        location2 = &buffer[location[x] + 3];
        table->Data[Counter].is_free = (u8) fast_atoi(location2);
        load_string(
                location[x+6] + 2,
                location[x+7] - 1,
                &buffer[location[x+2] + 2],
                sizeof(table->Data[Counter].type),
                table->Data[Counter].type);

        Counter++;
    }
    for (u32 x = 0; x < 1; x++) {
        load_string(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);
        load_string(
                location[x] + 2,
                location[x+1] - 1,
                &buffer[location[x] + 2],
                sizeof(table->Column[1]),
                table->Column[1]);
        load_string(
                location[x+1] + 2,
                location[x+2] - 1,
                &buffer[location[x+1] + 2],
                sizeof(table->Column[2]),
                table->Column[2]);
        load_string(
                location[x+2] + 2,
                location[x+3] - 1,
                &buffer[location[x+2] + 2],
                sizeof(table->Column[3]),
                table->Column[3]);
        load_string(
                location[x+5] + 2,
                location[x+6] - 1,
                &buffer[location[x+5] + 2],
                sizeof(table->Column[4]),
                table->Column[4]);
    }
}

void thread_info_find(
        games * table,
        u32 SizeArray,
        char * buffer,
        u32 * locations,
        u8 TotalThreads,
        thread_info * Info
        ) {
    u32 division_size = SizeArray / TotalThreads;
    u8 extra = SizeArray - (division_size * TotalThreads);
    for (u8 x = 0; x < TotalThreads; x++) {
        Info[x].SizeArray = SizeArray;
        Info[x].table = table;
        Info[x].buffer = buffer;
        Info[x].location = locations;
        Info[x].WorkStartIndex = division_size + division_size * x;
        Info[x].division_size = division_size;
    }
    Info[TotalThreads-1].extra = extra;
}
void game_parse_multithread(
        thread_info * Info) {
    u32 Counter = 0;
    u32 StartIndex = Info->WorkStartIndex;
    u32 WorkEndIndex = StartIndex + Info->division_size + Info->extra;
    char * location1 = 0;
    char * location2 = 0;
    for (u32 x = StartIndex; x < WorkEndIndex; x += 7) {
        location1 = &(Info->buffer[Info->location[x] + 2]);
        Info->table->Data[Counter].id = fast_atoi(location1);
        load_string(
                Info->location[x+1] + 2,
                Info->location[x+2] - 1,
                &(Info->buffer[Info->location[x+1] + 2]),
                sizeof(Info->table->Data[Counter].name),
                Info->table->Data[Counter].name);
        load_string(
                Info->location[x+2] + 2,
                Info->location[x+3] - 1,
                &Info->buffer[Info->location[x+2] + 2],
                sizeof(Info->table->Data[Counter].release_date),
                Info->table->Data[Counter].release_date);
        location2 = &Info->buffer[Info->location[x] + 3];
        Info->table->Data[Counter].is_free = (u8) fast_atoi(location2);
        load_string(
                Info->location[x+6] + 2,
                Info->location[x+7] - 1,
                &Info->buffer[Info->location[x+2] + 2],
                sizeof(Info->table->Data[Counter].type),
                Info->table->Data[Counter].type);

        Counter++;
    }
}

void game_column_parse(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location) {
    for (u32 x = 0; x < 1; x++) {
        load_string(
                1,
                location[x] - 1,
                &buffer[1],
                sizeof(table->Column[0]),
                table->Column[0]);
        load_string(
                location[x] + 2,
                location[x+1] - 1,
                &buffer[location[x] + 2],
                sizeof(table->Column[1]),
                table->Column[1]);
        load_string(
                location[x+1] + 2,
                location[x+2] - 1,
                &buffer[location[x+1] + 2],
                sizeof(table->Column[2]),
                table->Column[2]);
        load_string(
                location[x+2] + 2,
                location[x+3] - 1,
                &buffer[location[x+2] + 2],
                sizeof(table->Column[3]),
                table->Column[3]);
        load_string(
                location[x+5] + 2,
                location[x+6] - 1,
                &buffer[location[x+5] + 2],
                sizeof(table->Column[4]),
                table->Column[4]);
    }
}
