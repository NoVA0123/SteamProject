/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#ifndef W__CODE_REPETITION_TESTER_H_
#define W__CODE_REPETITION_TESTER_H_

#include <stdio.h>
#include "typedef.h"
#include "platform_metrics.cpp"

struct time_data {
    f64 MinTime;
    f64 MaxTime;
    u64 MaxClocks;
    u64 MinClocks;
    u64 TotalCount;
    u64 TotalClocks;
    u64 MaxPageFault;
    u64 MinPageFault;
};

struct clocks_recorder {
    u64 BeginTSC;
    u64 EndTSC;
    struct time_data Record;
    f64 CurrentMinTime;
    u64 TotalPageFault;
    u64 CurPageFault;
};

static struct clocks_recorder GlobalRecord;

void BeginTesterForTheFunction(void);

void EndTheTester(void);

u8 EvaluateTest(char const * Name, u64 ByteCount, u64 PageFault);

u8 EvaluateTestNoPageFault(char const * Name, u64 ByteCount);

u8 EvaluateTestNoPageFaultAndNoSpeed(char const * Name);

#endif  // W__CODE_REPETITION_TESTER_H_
