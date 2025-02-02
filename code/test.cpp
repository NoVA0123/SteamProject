/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


// #include <windows.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <math.h>
#include <windows.h>
#include <winbase.h>

#include "typedef.h"
#include "csv_parser.h"
#include "platform_metrics.cpp"
#include "nested_profiler.cpp"

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))
#define PROFILER 1

char * read_file(
        struct stat Stat,
        FILE *ptr) {
    TimeBlock(__func__, Stat.st_size);
    char * buffer = (char *) malloc(Stat.st_size);
    fread(buffer, sizeof(char), Stat.st_size, ptr);
    return buffer;
}
u32 FindSpecialCharacterIndex(char * buffer, u32 * location, struct stat Stat) {
    TimeFunction;
    u32 SizeArray = find_values_from_buffer(buffer, location, Stat.st_size);
    return SizeArray;
}

void ParseCSVTimedFunction(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location
        ) {
    TimeFunction;
    game_parse(table, SizeArray, buffer, location);
}

int main() {
    BeginRecord();
    FILE *ptr = fopen("games.csv", "rb");
    struct stat Stat = file_size("games.csv");
    static char* buffer;

    static u32 * location = (u32*) malloc(sizeof(u32) * 980608);

    buffer = read_file(Stat, ptr);
    // fread(buffer, sizeof(char), Stat.st_size, ptr);

    u32 SizeArray = FindSpecialCharacterIndex(buffer,location,Stat);
    // u32 SizeArray = find_values_from_buffer(buffer, location, Stat.st_size);

    static struct GameColumns * new_data;
    struct games table;

    new_data = (struct GameColumns *) malloc(sizeof(struct GameColumns) *
            (SizeArray / 7));
    table.Data = new_data;

    ParseCSVTimedFunction(&table, SizeArray, buffer,location);
    // game_parse(&table, SizeArray, buffer, location);

    printf("%p\n", &table.Data[140080].release_date);
    printf("%p\n", &table.Column[4]);
    printf("%s\n", table.Column[4]);

    char Stringbuffer[256];
    sprintf(Stringbuffer, "%p\n", &table.Data[140080].release_date);
    OutputDebugString(Stringbuffer);
    sprintf(Stringbuffer, "%p\n", &table.Column[4]);
    OutputDebugString(Stringbuffer);
    sprintf(Stringbuffer, "%s\n", table.Column[4]);
    OutputDebugString(Stringbuffer);

    EndAndPrintAll();
    free(buffer);
    free(location);
    free(new_data);
    return 0;
}
