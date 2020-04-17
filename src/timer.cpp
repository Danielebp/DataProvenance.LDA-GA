#include "timer.h"

using namespace std;


Timer::Timer(){
}

bool Timer::start(){
    t_start = chrono::high_resolution_clock::now();
}
bool Timer::reset(){
    t_start = chrono::high_resolution_clock::now();
}
long Timer::getTime(){
    t_end = chrono::high_resolution_clock::now();
    auto delta = t_end - t_start;
    return chrono::duration_cast<chrono::milliseconds>(delta).count();
}


