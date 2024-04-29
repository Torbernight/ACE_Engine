#pragma once

#include <chrono>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;
typedef std::chrono::_V2::system_clock::time_point time_point;

inline static
time_point get_time()
{
    return Time::now();
}

struct Timer
{
    time_point T0, T1;
    fsec T;
    float dt;

    void Start()
    {
        T0 = get_time();
    }

    float Count()
    {
        T1 = get_time();
        T = (T1 - T0);
        return T.count();
    }
};