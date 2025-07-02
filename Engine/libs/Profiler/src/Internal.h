#pragma once

int getValue();

// (C) 2025 DigiPen (USA) Corporation

#pragma once
#include <string>

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)
#pragma intrinsic(_mm_mfence)
#pragma intrinsic(_mm_lfence)
#pragma intrinsic(__rdtsc)
#endif


namespace sync_engine
{

    inline void* get_return_address()
    {
#ifdef _MSC_VER
        return _ReturnAddress();
#else
        return __builtin_return_address(0);
#endif
    }


    /* CLOCK CYCLE COUNTING
     *
     * By calling mfence/lfence, we can prevent out-of-order execution around the function boundaries,
     * thus can get more accurate cycle counts.
     * [Reference: "Intel 64 and IA-32 Architectures Software Developer¡¯s Manual"]
     *
     * For really old processors, clock cycle may be affected by turbo boost or any other features that change the clock speed.
     *      For Pentium M processors (family [06H], models [09H, 0DH]); for Pentium 4 processors, Intel Xeon processors
     *      (family [0FH], models [00H, 01H, or 02H]); and for P6 family processors
     * For modern processors, the clock cycle is constant.
     * [Reference: "Intel 64 and IA-32 Architectures Software Developer¡¯s Manual"]
     * Here, I will not consider the old processors.
     *
     * If the computer has multiple physical CPUs(multi-socket), the TSCs of each CPU *might* not be synchronized.
     * Here, I will not consider the multi-socket case.
     *
     * Since software can modify the TSC, it can be out-of-sync across different (logical) processors.
     * To be really cautious, we need to calibrate the TSC before starting the measurement.
     *      [Can refer to the "Time-Stamp Counter Adjustment" section in Intel's Manual]
     * Here, I will not consider the case where the TSC is modified unevenly.
     */


    inline unsigned long long get_tsc_enter()
    {
#ifdef _MSC_VER
        // If software requires RDTSC to be executed only after all previous instructions have executed and all previous
        // loads and stores are globally visible, it can execute the sequence MFENCE; LFENCE immediately before RDTSC.
        _mm_mfence();
        _mm_lfence();
        return __rdtsc();
#else
#endif
    }


    inline unsigned long long get_tsc_exit()
    {
#ifdef _MSC_VER
        // If software requires RDTSC to be executed prior to execution of any subsequent instruction (including any
        // memory accesses), it can execute the sequence LFENCE immediately after RDTSC.
        const unsigned long long tsc = __rdtsc();
        _mm_lfence();
        return tsc;
#else
#endif
    }


    std::string get_function_name(void* address);

}