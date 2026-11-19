#pragma once

#include <chrono>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float> fsec;
typedef std::chrono::_V2::system_clock::time_point TimePoint;

struct Timer
{
    TimePoint T0, T1;
    fsec T;
    float dt;

    void Start()
    {
        T0 = Time::now();
    }

    float Count()
    {
        T1 = Time::now();
        T = (T1 - T0);
        return T.count();
    }
};