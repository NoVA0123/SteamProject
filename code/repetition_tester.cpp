/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <stdio.h>
#include "typedef.h"
#include "repetition_tester.h" 


void BeginTesterForTheFunction(void) {
    GlobalRecord.BeginTSC = read_cpu_timer();
}

void EndTheTester(void) {
    GlobalRecord.EndTSC = read_cpu_timer();
}

//TODO(Abhijit): Run the begin then end the TSC after that record min and max and time for current min
u8 EvaluateTest(char const * Name, u64 ByteCount, u64 PageFault) {
    u64 TotalClocks = GlobalRecord.EndTSC - GlobalRecord.BeginTSC;
    u64 CPUFREQ = estimate_cpu_timer_freq();
    f64 TimeTaken = (f64)TotalClocks / (f64)CPUFREQ;

    GlobalRecord.CurPageFault = PageFault -
        GlobalRecord.CurPageFault;

    GlobalRecord.CurrentMinTime += TimeTaken;
    GlobalRecord.Record.TotalClocks += TotalClocks;
    GlobalRecord.Record.TotalCount++;
    GlobalRecord.TotalPageFault += GlobalRecord.CurPageFault;

    if ((TimeTaken < GlobalRecord.Record.MinTime) || (GlobalRecord.Record.MinTime == 0)) {
        GlobalRecord.Record.MinTime = TimeTaken;
        GlobalRecord.CurrentMinTime = 0.0f;
        GlobalRecord.Record.MinPageFault = GlobalRecord.CurPageFault;
    } else if (TimeTaken > GlobalRecord.Record.MaxTime) {
        GlobalRecord.Record.MaxTime = TimeTaken;
        GlobalRecord.Record.MaxPageFault = GlobalRecord.CurPageFault;
    }

    if ((GlobalRecord.CurrentMinTime > 10.0f) || (GlobalRecord.Record.TotalCount == 1)) {
        f64 MegaByte = 1024.0f * 1024.0f;
        f64 GigaByte = 1024.0f * MegaByte;

        f64 MaxBytesPerSecond = (f64)ByteCount / GlobalRecord.Record.MinTime;
        f64 MaxGBPerSecond = MaxBytesPerSecond / GigaByte;

        f64 MinBytesPerSecond = (f64)ByteCount / GlobalRecord.Record.MaxTime;
        f64 MinGBPerSecond = MinBytesPerSecond / GigaByte;

        f64 AvgTime = ((f64)GlobalRecord.Record.TotalClocks / 
            (f64)GlobalRecord.Record.TotalCount) / (f64)CPUFREQ;
        f64 AvgBytesPerSecond = (f64)ByteCount / AvgTime;
        f64 AvgGBPerSecond = AvgBytesPerSecond / GigaByte;

        f64 SizePerPageFaultForMin = (f64)ByteCount /
            (f64) GlobalRecord.Record.MinPageFault;
        f64 SizePerPageFaultForMax = (f64)ByteCount /
            (f64) GlobalRecord.Record.MaxPageFault;

        f64 AvgFault = (f64) GlobalRecord.TotalPageFault /
            (f64) GlobalRecord.Record.TotalCount;
        f64 SizePerPageFaultForAvg = (f64) ByteCount / AvgFault;

        printf("%s\n", Name);
        printf("%llu actual %llu\n", GlobalRecord.CurPageFault, PageFault);

        printf("Maximum Time: %f at %.2fgb/s, PF: %llu (%0.3fk/fault)\n",
                GlobalRecord.Record.MaxTime,
                MinGBPerSecond,
                GlobalRecord.Record.MaxPageFault,
                SizePerPageFaultForMax);

        printf("Minimum Time: %f at %.2fgb/s, PF: %llu (%0.3fk/fault)\n",
                GlobalRecord.Record.MinTime,
                MaxGBPerSecond,
                GlobalRecord.Record.MinPageFault,
                SizePerPageFaultForMin);
        printf("Average Time: %f at %.2fgb/s, PF: %0.2f (%0.3fk/fault)\n",
                AvgTime,
                AvgGBPerSecond,
                AvgFault,
                SizePerPageFaultForAvg);
        return 1;
    }
    return 0;
}

u8 EvaluateTestNoPageFault(char const * Name, u64 ByteCount) {
    u64 TotalClocks = GlobalRecord.EndTSC - GlobalRecord.BeginTSC;
    u64 CPUFREQ = estimate_cpu_timer_freq();
    f64 TimeTaken = (f64)TotalClocks / (f64)CPUFREQ;

    GlobalRecord.CurrentMinTime += TimeTaken;
    GlobalRecord.Record.TotalClocks += TotalClocks;
    GlobalRecord.Record.TotalCount++;

    if ((TimeTaken < GlobalRecord.Record.MinTime) || (GlobalRecord.Record.MinTime == 0)) {
        GlobalRecord.Record.MinTime = TimeTaken;
        GlobalRecord.CurrentMinTime = 0.0f;
        GlobalRecord.Record.MinClocks = TotalClocks;
    }
    if (TimeTaken > GlobalRecord.Record.MaxTime) {
        GlobalRecord.Record.MaxTime = TimeTaken;
        GlobalRecord.Record.MaxClocks = TotalClocks;
    }

    if ((GlobalRecord.CurrentMinTime > 10.0f) || (GlobalRecord.Record.TotalCount == 10)) {
        f64 MegaByte = 1024.0f * 1024.0f;
        f64 GigaByte = 1024.0f * MegaByte;

        f64 MaxBytesPerSecond = (f64)ByteCount / GlobalRecord.Record.MinTime;
        f64 MaxGBPerSecond = MaxBytesPerSecond / GigaByte;

        f64 MinBytesPerSecond = (f64)ByteCount / GlobalRecord.Record.MaxTime;
        f64 MinGBPerSecond = MinBytesPerSecond / GigaByte;

        f64 AvgClocks = (f64) GlobalRecord.Record.TotalClocks /
            (f64) GlobalRecord.Record.TotalCount;
        f64 AvgTime = AvgClocks / (f64) CPUFREQ;
        f64 AvgBytesPerSecond = (f64)ByteCount / AvgTime;
        f64 AvgGBPerSecond = AvgBytesPerSecond / GigaByte;

        printf("%s\n", Name);

        printf("Maximum Time: %f at %.2fgb/s %llu\n",
                GlobalRecord.Record.MaxTime,
                MinGBPerSecond,
                GlobalRecord.Record.MaxClocks);

        printf("Minimum Time: %f at %.2fgb/s %llu\n",
                GlobalRecord.Record.MinTime,
                MaxGBPerSecond,
                GlobalRecord.Record.MinClocks);

        printf("Average Time: %f at %.2fgb/s %.2f\n",
                AvgTime,
                AvgGBPerSecond,
                AvgClocks);
        GlobalRecord = {};
        return 1;
    }
    return 0;
}

u8 EvaluateTestNoPageFaultAndNoSpeed(char const * Name) {
    u64 TotalClocks = GlobalRecord.EndTSC - GlobalRecord.BeginTSC;
    u64 CPUFREQ = estimate_cpu_timer_freq();
    f64 TimeTaken = (f64)TotalClocks / (f64)CPUFREQ;

    GlobalRecord.CurrentMinTime += TimeTaken;
    GlobalRecord.Record.TotalClocks += TotalClocks;
    GlobalRecord.Record.TotalCount++;

    if ((TimeTaken < GlobalRecord.Record.MinTime) || (GlobalRecord.Record.MinTime == 0)) {
        GlobalRecord.Record.MinTime = TimeTaken;
        GlobalRecord.CurrentMinTime = 0.0f;
        GlobalRecord.Record.MinClocks = TotalClocks;
    }
    if (TimeTaken > GlobalRecord.Record.MaxTime) {
        GlobalRecord.Record.MaxTime = TimeTaken;
        GlobalRecord.Record.MaxClocks = TotalClocks;
    }

    if ((GlobalRecord.CurrentMinTime > 10.0f) || (GlobalRecord.Record.TotalCount == 10)) {
        f64 AvgClocks = (f64) GlobalRecord.Record.TotalClocks /
            (f64) GlobalRecord.Record.TotalCount;
        f64 AvgTime = AvgClocks / (f64) CPUFREQ;

        printf("%s\n", Name);

        printf("Maximum Time: %f at %lluclocks\n",
                GlobalRecord.Record.MaxTime,
                GlobalRecord.Record.MaxClocks);

        printf("Minimum Time: %f at %lluclocks\n",
                GlobalRecord.Record.MinTime,
                GlobalRecord.Record.MinClocks);

        printf("Average Time: %f at %.2fclocks\n",
                AvgTime,
                AvgClocks);
        GlobalRecord = {};
        return 1;
    }
    return 0;
}
