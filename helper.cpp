#include <iostream>
#include <map>
#include <vector>
#include "Events.hpp"
#include <string>
#include <chrono>
#include "helper.hpp"
#include <mutex>


Timer::Timer(){
    is_running = false;
}

void Timer::start(){
    start_time = std::chrono::high_resolution_clock::now();
    is_running = true;
}

void Timer::stop(){
    is_running = false;
}

double Timer:: time_elapsed(std::string units, std::string s){
    if (is_running) {
            return 0.0;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        if (units == "microsecond"){
            std::cout << s <<" Elapsed time: " << duration.count() << " microseconds" << std::endl;
            return duration.count();
        }
        else if (units == "ms"){
            std::cout << s <<" Elapsed time: " << duration.count() / 1000.0 << " ms" << std::endl;
            return duration.count() / 1000.0;
        }
        else if (units == "s"){
            std::cout << s <<" Elapsed time: " << duration.count() / 1000000.0 << " s" << std::endl;
            return duration.count() / 1000000.0;
        }
        else{
            std::cout << s << " Elapsed time: " << duration.count() / 60.0 / 1000000.0 << " minutes" << std::endl;
            return duration.count()/ 60.0 / 1000000.0;
        }
}

template <class T> void releaseVectorMemory(std::vector<T>& vec) {
    std::vector<T>().swap(vec); 
}

MarketEvent get_market_event(std::vector<float> raw_market_data){
    MarketEvent me(0, 0, 0, 0, 0, 0);
    me.instrument_id = raw_market_data[5];
    me.ms_of_day = raw_market_data[0];
    me.bid = raw_market_data[2];
    me.bid_size = raw_market_data[1];
    me.ask = raw_market_data[4];
    me.ask_size = raw_market_data[3];

    return me;
}
