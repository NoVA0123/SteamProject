/* Copyright[2024] Abhijit Rai */
#include <stdint.h>
typedef uint64_t u64;


#if _WIN64


#include <intrin.h>
#include <windows.h>
#include <profileapi.h>
#include <winnt.h>


static u64 get_os_time_freq(void) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

static u64 read_os_timer(void) {
    LARGE_INTEGER Value;
    QueryPerformanceCounter(&Value);
    return Value.QuadPart;
}


#else


#include <x86intrin.h>
#include <sys/time.h>


static u64 get_os_time_freq(void) {
    return 1000000;
}



static u64 read_os_timer(void) {
    struct timeval Value;
    gettimeofday(&Value, 0);
    u64 Result = GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
    return Result;
}


#endif


static inline u64 read_cpu_timer(void) {
    return _rdtsc();
}


static u64 estimate_cpu_timer_freq(void) {
    u64 MsToWait = 100;
    u64 OsFreq = get_os_time_freq();
    u64 CpuStart = read_cpu_timer();
    u64 OsStart = read_os_timer();
    u64 OsEnd = 0;
    u64 OsElapsed = 0;
    u64 OsWaitTime = OsFreq * MsToWait / 1000;
    while (OsElapsed < OsWaitTime) {
        OsEnd = read_os_timer();
        OsElapsed = OsEnd - OsStart;
    }
    u64 CpuEnd = read_cpu_timer();
    u64 CpuElapsed = CpuEnd - CpuStart;
    u64 CpuFreq = 0;
    if (OsElapsed) {
        CpuFreq = OsFreq * CpuElapsed / OsElapsed;
    }
    return CpuFreq;
}
