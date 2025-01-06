#include "Helper.hpp"


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

MarketEvent get_market_event(const int instrument_id, const int ms_of_day, const std::array<float, 4> raw_market_data){
    MarketEvent me(instrument_id,ms_of_day, raw_market_data[0], raw_market_data[1], raw_market_data[2], raw_market_data[3]);
    return me;
}

std::string time_increment_one_days(const std::string cur_date){
    int cur_date_int = std::stoi(cur_date);

    int y = cur_date_int / 10000;
    cur_date_int -= y * 10000;

    int m = cur_date_int / 100;
    cur_date_int -= m * 100;   

    int d = cur_date_int / 1;

    int last_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    // Leap Year
    if (y % 4 == 0){
        last_day[1] = 29;
    } 

    d += 1;

    if (d > last_day[m-1]){
        m += 1;
    }

    if (m > 12){
        m = 1;
        y += 1;
    }

    int next_date_int = y * 10000 + m * 100 + d;
    return std::to_string(next_date_int);

}