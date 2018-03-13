// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "rpc/timer/timer_utilties.h"
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "log/logging.h"

int64_t Clock::CurrentTimeMillis()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

std::string Clock::CurrentTimeString(int64_t timepoint)
{
    time_t sec = timepoint / 1000;
    struct tm* pinfo = localtime(&sec); // make sure localtime has thread-safety
    char buffer[100] = {};
    int n = snprintf(buffer, 100, "%d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + pinfo->tm_year, pinfo->tm_mon + 1,
        pinfo->tm_mday, pinfo->tm_hour, pinfo->tm_min, pinfo->tm_sec, int(timepoint % 1000));
    return std::string(buffer, n);
}

uint64_t Clock::GetNowTicks()
{
#ifdef _WIN32
    uint64_t freq = 0;
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
    {
        LOG(FATAL) << GetLastError();
    }
    uint64_t now = 0;
    if (!QueryPerformanceCounter((LARGE_INTEGER*)&now))
    {
        LOG(FATAL) << GetLastError();
    }
    return (now * 1000000000UL) / freq;
#else
    timespec ts;
    //if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
    {
        LOG(FATAL) << "error!";
    }
    return (ts.tv_sec * 1000000000UL) + ts.tv_nsec;
#endif
}

struct timespec Clock::TransTicks(uint64_t ticks) {
    struct timespec t;
    t.tv_sec = ticks / 1000000000UL;
    t.tv_nsec = ticks % 1000000000UL; 
    return t;
}
