#ifndef __timer_h__
#define __timer_h__

#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

class Timer {

    time_point<high_resolution_clock> t_start;
    time_point<high_resolution_clock> t_end;

public:
    Timer();
    bool start();
    bool reset();
    long getTime();

};

#endif
