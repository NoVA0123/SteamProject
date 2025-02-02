/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#ifndef W__CODE_CSV_PARSER_H_
#define W__CODE_CSV_PARSER_H_
#include <sys/stat.h>
#include <math.h>
#include <windows.h>
#include <winbase.h>
#include <processthreadsapi.h>
#include "typedef.h"


struct stat file_size(char *pfileName);

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
        u64 size);

void game_parse(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void thread_info_find(
        struct games * table,
        u32 SizeArray,
        char * buffer,
        u32 * locations,
        u8 TotalThreads,
        struct thread_info * Info);

void game_parse_multithread(
        struct thread_info * Info);

void game_column_parse(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

#endif  // W__CODE_CSV_PARSER_H_
