/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <sys/stat.h>
#include <math.h>

#include "typedef.h"
#include "csv_parser.h"
#include "platform_metrics.cpp"


int main() {
    FILE *ptr = fopen("test.csv", "rb");
    struct stat Stat = file_size("test.csv");
    static char* buffer;
    buffer = (char*) malloc(Stat.st_size);

    u64 FreadStart = read_cpu_timer();
    fread(buffer, sizeof(char), Stat.st_size, ptr);
    u64 FreadClocks = read_cpu_timer() - FreadStart;

    u64 AllocStart = read_cpu_timer();
    static char ***Columns = allocate_table_memory(10, 10, 200);
    u64 AllocClocks = read_cpu_timer() - AllocStart;

    u64 ParseStart = read_cpu_timer();
    Columns = parse_csv(buffer, Columns, Stat);
    u64 ParseClocks = read_cpu_timer() - ParseStart;

    free(buffer);

    u64 CpuFreq = estimate_cpu_timer_freq();
    u64 TotalClocks = FreadClocks + AllocClocks + ParseClocks;

    u32 newbuffer[10];
    for (u8 x = 0; x < 10; x++) {
        char bruh[] = "KJKLJKLJKLJ";
        newbuffer[x] =(u32) bruh;
        printf("Pointer: %d\n", newbuffer[x]);
    }

    printf("\nTotal CPU Clocks: %llu\n", CpuFreq);

    printf("Total time and clocks: %0.4f", (f64)TotalClocks / (f64)CpuFreq);
    printf(" (%llu)\n", TotalClocks);

    printf("Total Fread time: %0.4f", (f64)FreadClocks / (f64)CpuFreq);
    printf(" (%llu)", FreadClocks);
    printf(" %0.4f%%\n", ((f64)FreadClocks / (f64)TotalClocks) * 100.00);

    printf("Total Allocate Buffer time: %0.4f", (f64)AllocClocks / (f64)CpuFreq);
    printf(" (%llu)", AllocClocks);
    printf(" %0.4f%%\n", ((f64)AllocClocks / (f64)TotalClocks) * 100.00);

    printf("Total Parse time: %0.4f", (f64)ParseClocks / (f64)CpuFreq);
    printf(" (%llu)", ParseClocks);
    printf(" %0.4f%%\n", ((f64)ParseClocks / (f64)TotalClocks) * 100.00);

    return 0;
}
