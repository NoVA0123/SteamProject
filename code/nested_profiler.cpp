/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <cstddef>
#include <stdio.h>
#include "typedef.h"
#include "platform_metrics.h"

// TODO(Abhijit): Create a struct that contains start time and end time.
struct profile {
    u64 StartCounter; 
    u64 TotalClocks;
};

struct anchor_profile {
    u64 TSCElapsedClocks;
    u64 HitCount;
    char * label;
    u64 ByteCount;
};

static anchor_profile GlobalAnchorProfile[10];
static profile GlobalProfile;
static u32 GlobalAnchorProfileParent;

// TODO(Abhijit): Create a struct that performs it task after the function end(use destructor).
struct profile_block {
    profile_block(char * label_, u64 ByteCount_) {
        CurProfile = GlobalAnchorProfile + GlobalAnchorProfileParent + 1;
        GlobalAnchorProfileParent++;
        Label = label_;
        StartTSC = read_cpu_timer();
        TotalBytes = ByteCount_;
    }

    ~profile_block(void) {
        CurProfile -> TSCElapsedClocks = read_cpu_timer() - StartTSC;
        ++(CurProfile -> HitCount);
        CurProfile -> label = Label;
        CurProfile -> ByteCount = TotalBytes;
    }
    char * Label;
    anchor_profile * CurProfile;
    u64 StartTSC;
    u64 TotalBytes;
};


// TODO(Abhijit): Create Macro to simplify the function performance.
#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name, ByteCount) profile_block NameConcat(Block, __LINE__)(Name, ByteCount);
#define TimeFunction TimeBlock(__func__, 0)
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(GlobalAnchorProfile), "Number of profile points exceeds size of profiler::Anchors array")

// TODO(Abhijit): Create a start function and print function.

static void PrintTimeElapsed(u64 TotalTSCElapsed, anchor_profile * Anchor) {
    f64 Percent = 100.0 * ((f64)Anchor -> TSCElapsedClocks / (f64) TotalTSCElapsed);
    u64 CPUFreq = estimate_cpu_timer_freq();
    printf("  %s[%llu]: %llu  %.2f%%",
            Anchor -> label,
            Anchor -> HitCount,
            Anchor -> TSCElapsedClocks,
            Percent);
    if (Anchor->ByteCount) {
        f64 MegaByte = 1024.0f * 1024.0f;
        f64 GigaByte = 1024.0f * MegaByte;

        f64 Seconds = (f64)Anchor->TSCElapsedClocks / (f64) CPUFreq;
        f64 BytesPerSeconds = (f64)Anchor->ByteCount / (f64) Seconds;
        f64 MegaBytes = (f64)Anchor -> ByteCount / (f64)MegaByte;
        f64 GigaBytesPerSecond = BytesPerSeconds / GigaByte;
        printf("  %.3fmb at %.2fgb/s", MegaBytes, GigaBytesPerSecond);
    }
    printf("\n");
}

static void BeginRecord(void) {
    GlobalProfile.StartCounter = read_cpu_timer();
}

static void EndAndPrintAll(void) {
    GlobalProfile.TotalClocks = read_cpu_timer() - GlobalProfile.StartCounter;
    u64 CPUFreq = estimate_cpu_timer_freq();
    f64 TotalTime = (f64)GlobalProfile.TotalClocks / (f64) CPUFreq;
    printf("TotalTime and Clocks: %.2f[%llu]\n", TotalTime, GlobalProfile.TotalClocks);
    for (u32 x = 1; x < ArrayCount(GlobalAnchorProfile); x++) {
        if (GlobalAnchorProfile[x].label == NULL) {
            break;
        }
        PrintTimeElapsed(GlobalProfile.TotalClocks, &GlobalAnchorProfile[x]);
    }
}
