#include <iostream>
#include <map>
#include <vector>
#include "Events.hpp"
#include <string>
#include <chrono>

#ifndef HELPER_H
#define HELPER_H


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
template <class T> void releaseVectorMemory(std::vector<T>& vec);


MarketEvent get_market_event(std::vector<float> raw_market_data);
#endif