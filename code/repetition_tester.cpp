/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <stdio.h>
#include "typedef.h"
#include "platform_metrics.h"


struct time_data {
    u64 MinTime;
    u64 MaxTime;
    u64 TotalCount;
    f64 TotalTime;
};

struct clocks_recorder {
    u64 BeginTSC;
    u64 EndTSC;
    time_data Record;
    char const * Label;
};

static clocks_recorder GlobalRecord[3];
static u32 GlobalIndex;

void BeginTesterForTheFunction() {
    ++GlobalIndex;
    GlobalRecord[GlobalIndex - 1].BeginTSC = read_cpu_timer();
}

void EndTheTester(char const * Name) {
    ++GlobalIndex;
    GlobalRecord[GlobalIndex - 1].EndTSC = read_cpu_timer();
    GlobalRecord[GlobalIndex - 1].Label = Name;
}

//TODO 
void DisplayResults(void) {
    for (u32)
}
