/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <cstdlib>
#include <windows.h>
#include <winbase.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <math.h>

#include "repetition_tester.h"
#include "typedef.h"
#include "csv_parser.h"
#include "combiner.h"

#define MAX_NUM_THREADS 4

#define PROFILER 1
static thread_info ThreadInfo[MAX_NUM_THREADS];
DWORD ThreadID[MAX_NUM_THREADS];
HANDLE Threads[MAX_NUM_THREADS];


static struct games gGamesTable;
static struct category gCategoryTable;
static struct genre gGenreTable;
static struct summary gSummaryTable;
static struct other gOtherTable;


enum FileNames {
    GamesFile,
    CategoryFile,
    GenreFile,
    SummaryFile,
    OtherFile,
};


void * GameParseInitialize(
        csv_info * Info,
        struct games * GamesTable
        ) {
    static struct games_columns * GamesData;
    u32 SizeArray = (Info -> uSizeArray - 1) / Info -> ubFirstLineFeed;
    GamesData = (struct games_columns *) malloc(sizeof(struct games_columns) *
            SizeArray);
    GamesTable -> Data = GamesData;
    GamesTable -> uSizeArray = SizeArray;
    void * Table = (void *)GamesTable;
    return Table;
}

void * CategoryParseInitialize(
        csv_info * Info,
        struct category * CategoryTable
        ) {
    static struct category_column * CategoryData;
    u32 SizeArray = (Info -> uSizeArray - 1) / Info -> ubFirstLineFeed;
    CategoryData = (struct category_column *) malloc(
            sizeof(struct category_column) * SizeArray);
    CategoryTable -> Data = CategoryData;
    CategoryTable -> uSizeArray = SizeArray;
    void * Table = (void *)CategoryTable;
    return Table;
}

void * GenreParseInitialize(
        csv_info * Info,
        struct genre * GenreTable
        ) {
    static struct genre_column * GenreData;
    u32 SizeArray = (Info -> uSizeArray - 1) / Info -> ubFirstLineFeed;
    GenreData = (struct genre_column *) malloc(
            sizeof(struct genre_column) * SizeArray);
    GenreTable -> Data = GenreData;
    GenreTable -> uSizeArray = SizeArray;
    void * Table = (void *)GenreTable;
    return Table;
}

void * SummaryParseInitialize(
        csv_info * Info,
        struct summary * SummaryTable
        ) {
    static struct summary_column * SummaryData;
    u32 SizeArray = (Info -> uSizeArray - 1) / Info -> ubFirstLineFeed;
    SummaryData = (struct summary_column *) malloc(
            sizeof(struct summary_column) * SizeArray);
    SummaryTable -> Data = SummaryData;
    SummaryTable -> uSizeArray = SizeArray;
    void * Table = (void *)SummaryTable;
    return Table;
}

void * OtherParseInitialize(
        csv_info * Info,
        struct other * OtherTable
        ) {
    static struct other_column * OtherData;
    u32 SizeArray = (Info -> uSizeArray - 1) / Info -> ubFirstLineFeed;
    OtherData = (struct other_column *) malloc(
            sizeof(struct other_column) * SizeArray);
    OtherTable -> Data = OtherData;
    OtherTable -> uSizeArray = SizeArray;
    void * Table = (void *)OtherTable;
    return Table;
}


void GameParse(
        u32 SizeArray,
        char * buffer,
        u32 * location
        ) {
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            Threads[x] = CreateThread(
                    0,
                    0,
                    GamesParseMultiThread,
                    &ThreadInfo[x],
                    0,
                    &ThreadID[x]);
        }
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            CloseHandle(Threads[x]);
        }

    GameColumnParse(
            &gGamesTable,
            SizeArray,
            buffer,
            location);
}

void CategoryParse(
        u32 SizeArray,
        char * buffer,
        u32 * location
        ) {
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            Threads[x] = CreateThread(
                    0,
                    0,
                    CategoryParseMultiThread,
                    &ThreadInfo[x],
                    0,
                    &ThreadID[x]);
        }
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            CloseHandle(Threads[x]);
        }
    CategoryColumnParse(
            &gCategoryTable,
            SizeArray,
            buffer,
            location);
}

void GenreParse(
        u32 SizeArray,
        char * buffer,
        u32 * location
        ) {
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            Threads[x] = CreateThread(
                    0,
                    0,
                    GenreParseMultiThread,
                    &ThreadInfo[x],
                    0,
                    &ThreadID[x]);
        }
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            CloseHandle(Threads[x]);
        }
    GenreColumnParse(
            &gGenreTable,
            SizeArray,
            buffer,
            location);
}

void SummaryParse(
        u32 SizeArray,
        char * buffer,
        u32 * location
        ) {
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            Threads[x] = CreateThread(
                    0,
                    0,
                    SummaryParseMultiThread,
                    &ThreadInfo[x],
                    0,
                    &ThreadID[x]);
        }
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            CloseHandle(Threads[x]);
        }
    SummaryColumnParse(
            &gSummaryTable,
            SizeArray,
            buffer,
            location);
}

void OtherParse(
        u32 SizeArray,
        char * buffer,
        u32 * location
        ) {
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            Threads[x] = CreateThread(
                    0,
                    0,
                    OtherParseMultiThread,
                    &ThreadInfo[x],
                    0,
                    &ThreadID[x]);
        }
        for (u8 x = 0; x < MAX_NUM_THREADS; x++) {
            CloseHandle(Threads[x]);
        }
    OtherColumnParse(
            &gOtherTable,
            SizeArray,
            buffer,
            location);
}


void FileParser(char * FileName, int TypeOfFile) {
    FILE *ptr = fopen(FileName, "rb");
    if (ptr == NULL) {
        perror("File is not read");
    }
    struct stat Stat = FileSize(FileName);
    char* Gamebuffer;

    u32 * location = (u32*) malloc(sizeof(u32) * (1048576 * 2));
    Gamebuffer = (char *) malloc(Stat.st_size);
    if(Gamebuffer == NULL) {
        printf("can't allocate buffer for %s\n", FileName);
    }

    fread(Gamebuffer, sizeof(char), Stat.st_size, ptr);

    csv_info TmpInfo = {};
    struct csv_info * Info = FindValuesFromBuffer(
            Gamebuffer,
            location,
            Stat.st_size,
            &TmpInfo);

    u32 SizeArray = Info -> uSizeArray;
    void * pTable;
    /*if (TypeOfFile == GamesFile) {
        pTable = GameParseInitialize(*Info, &gGamesTable);
    } else if (TypeOfFile == CategoryFile) {
        pTable = CategoryParseInitialize(*Info, &gCategoryTable);
    } else if (TypeOfFile == GenreFile) {
        pTable = GenreParseInitialize(*Info, &gGenreTable);
    } else if (TypeOfFile == SummaryFile) {
        pTable = SummaryParseInitialize(*Info, &gSummaryTable);
    } else if (TypeOfFile == OtherFile) {
        pTable = OtherParseInitialize(*Info, &gOtherTable);
    }*/
    switch (TypeOfFile) {
        case GamesFile:
            pTable = GameParseInitialize(Info, &gGamesTable);
            break;
        case CategoryFile:
            pTable = CategoryParseInitialize(Info, &gCategoryTable);
            break;
        case GenreFile:
            pTable = GenreParseInitialize(Info, &gGenreTable);
            break;
        case SummaryFile:
            pTable = SummaryParseInitialize(Info, &gSummaryTable);
            break;
        case OtherFile:
            pTable = OtherParseInitialize(Info, &gOtherTable);
            break;
    }

    ThreadInitializer(
            pTable,
            Gamebuffer,
            location,
            MAX_NUM_THREADS,
            ThreadInfo,
            Info);

    /*if (TypeOfFile == GamesFile) {
        GameParse(SizeArray, Gamebuffer, location, pTable);
    } else if (TypeOfFile == CategoryFile) {
        CategoryParse(SizeArray, Gamebuffer, location, pTable);
    }*/

    switch (TypeOfFile) {
        case GamesFile:
            GameParse(SizeArray, Gamebuffer, location);
            break;
        case CategoryFile:
            CategoryParse(SizeArray, Gamebuffer, location);
            break;
        case GenreFile:
            GenreParse(SizeArray, Gamebuffer, location);
            break;
        case SummaryFile:
            SummaryParse(SizeArray, Gamebuffer, location);
            break;
        case OtherFile:
            OtherParse(SizeArray, Gamebuffer, location);
            break;
    }
}

int main() {
    u64 StartMultiThreadGames = read_cpu_timer();
    FileParser(
            "/Data/games.csv",
            GamesFile);
    u64 EndMultiThreadGames = read_cpu_timer() - StartMultiThreadGames;

    u64 StartMultiThreadCategory = read_cpu_timer();
    FileParser(
            "/Data/categories.csv",
            CategoryFile);
    u64 EndMultiThreadCategory = read_cpu_timer() - StartMultiThreadCategory;

    u64 StartMultiThreadGenre = read_cpu_timer();
    FileParser(
            "/Data/genres.csv",
            GenreFile);
    u64 EndMultiThreadGenre = read_cpu_timer() - StartMultiThreadGenre;

    u64 StartMultiThreadSummary = read_cpu_timer();
    FileParser(
            "/Data/descriptions.csv",
            SummaryFile);
    u64 EndMultiThreadSummary = read_cpu_timer() - StartMultiThreadSummary;

    u64 StartMultiThreadOther = read_cpu_timer();
    FileParser(
            "/Data/steamspy_insights.csv",
            OtherFile);
    u64 EndMultiThreadOther = read_cpu_timer() - StartMultiThreadOther;

    static struct combined * pAllDataCombined;
    pAllDataCombined = (struct combined *) malloc(sizeof(struct combined)
            * gGamesTable.uSizeArray);
    GameCombine(pAllDataCombined, &gGamesTable);

    u64 CPUFREQ = estimate_cpu_timer_freq();
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
            MultiThreadTimeOther);

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

    printf("%u\n", gGamesTable.uSizeArray);
    printf("%u\n", gCategoryTable.uSizeArray);
    printf("%u\n", gGenreTable.uSizeArray);
    printf("%u\n", gSummaryTable.uSizeArray);
    printf("%u\n", gOtherTable.uSizeArray);

    free(gGamesTable.Data);
    free(gCategoryTable.Data);
    free(gGenreTable.Data);
    free(gSummaryTable.Data);
    free(gOtherTable.Data);

    return 0;
}
