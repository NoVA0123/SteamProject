
/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#ifndef W__CODE_PLATFORM_METRICS_H_
#define W__CODE_PLATFORM_METRICS_H_

#include "typedef.h"

#if _WIN64


#include <intrin.h>
#include <windows.h>
#include <profileapi.h>
#include <winnt.h>


u64 get_os_time_freq(void);
u64 read_os_timer(void);

#else


#include <x86intrin.h>
#include <sys/time.h>


u64 get_os_time_freq(void);
u64 read_os_timer(void);

#endif

u64 read_cpu_timer(void);
u64 estimate_cpu_timer_freq(void);


#endif  // W__CODE_PLATFORM_METRICS_H_
