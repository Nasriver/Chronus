#include <iostream>
#include <map>
#include <vector>
#include "Events.hpp"
#include <string>
#include <chrono>
#include <highfive/highfive.hpp>


#ifndef HELPER_H
#define HELPER_H

// Timer
class Timer{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    bool is_running;

public:
    void start();
    void stop();
    double time_elapsed(std::string units, std::string s);
    Timer();
};

// Memory Management
template <class T> void releaseVectorMemory(std::vector<T>& vec);

// Market Event Conversion
MarketEvent get_market_event(const int instrument_id, const int ms_of_day, const std::array<float, 4> raw_market_data);


// Time Increment
std::string time_increment_one_days(const std::string cur_date);
#endif

