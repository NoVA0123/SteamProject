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


struct stat FileSize(char *pfileName);

struct csv_info {
    u32 uSizeArray;
    u8 ubFirstLineFeed;
};

struct thread_info {
    void *table;
    u32 uSizeArray;
    char * caBuffer;
    u32 * uaLocation;
    u32 uStepSize;
    u8 ThreadIndex;
    u8 Extra;
    u8 ubFirstLineFeed;
};

struct games_columns{
    u32 uID;
    char sName[32];
    char sReleaseDate[16];
    u8 ubIsFree;
    char sType[8];
};

struct category_column{
    u32 uID;
    char sCategory[32];
};

struct genre_column{
    u32 uID;
    char sGenre[64];
};

struct summary_column{
    u32 uID;
    char sSummary[512];
};

struct other_column{
    u32 uID;
    char sDeveloper[32];
    char sPublisher[32];
    float fPrice;
    char sLanguage[128];
};

struct games{
    struct games_columns * Data;
    char Column[5][13];
    u32 uSizeArray;
};

struct category{
    struct category_column * Data;
    char Column[2][13];
    u32 uSizeArray;
};

struct genre{
    struct genre_column * Data;
    char Column[2][13];
    u32 uSizeArray;
};

struct summary{
    struct summary_column * Data;
    char Column[2][13];
    u32 uSizeArray;
};

struct other{
    struct other_column * Data;
    char Column[5][13];
    u32 uSizeArray;
};

struct csv_info * GPTFindValuesFromBuffer(
        char * buffer,
        u32 * Values,
        u64 size,
        struct csv_info * GameInfo);

struct csv_info * FindValuesFromBuffer(
        char * buffer,
        u32 * Values,
        u64 size,
        struct csv_info * GameInfo);

void GamesParseSingleThread(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void CategoryParseSingleThread(
        struct category *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void ThreadInitializer(
        void * table,
        char * buffer,
        u32 * locations,
        u8 TotalThreads,
        struct thread_info * Info,
        struct csv_info * GameInfo);

DWORD WINAPI GamesParseMultiThread(
        LPVOID lpParameter);

DWORD WINAPI CategoryParseMultiThread(
        LPVOID lpParameter);

DWORD WINAPI GenreParseMultiThread(
        LPVOID lpParameter);

DWORD WINAPI SummaryParseMultiThread(
        LPVOID lpParameter);

DWORD WINAPI OtherParseMultiThread(
        LPVOID lpParameter);

void GameColumnParse(
        struct games *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void CategoryColumnParse(
        struct category *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void SummaryColumnParse(
        struct summary *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void GenreColumnParse(
        struct genre *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

void OtherColumnParse(
        struct other *table,
        u32 SizeArray,
        char * buffer,
        u32 * location);

#endif  // W__CODE_CSV_PARSER_H_
