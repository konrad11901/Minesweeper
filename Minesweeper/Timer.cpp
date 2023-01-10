#include "pch.h"
#include "Timer.h"

Timer::Timer() : frequency(0), start() {}

void Timer::StartCounter() {
    LARGE_INTEGER curr_val;
    QueryPerformanceFrequency(&curr_val);

    frequency = double(curr_val.QuadPart) / 1000.0;

    QueryPerformanceCounter(&curr_val);
    start = curr_val;
}

double Timer::GetTime() {
    LARGE_INTEGER curr_counter;
    QueryPerformanceCounter(&curr_counter);

    return double(curr_counter.QuadPart - start.QuadPart) / frequency;
}