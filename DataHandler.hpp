#include <vector>
#include <deque>
#include <string>
#include <highfive/highfive.hpp>
#include "Events.hpp"
#include <mpi.h>
#include <map>
#include <format>
#include "Helper.hpp"
#include <array>
#include <limits>
#include <filesystem>
#include "Participant.hpp"
#include <cmath>

#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

class DataHandler: public Base_Participant{
    private:

        // DataHandler Info
        int datahandler_rank;
        int datahandler_size;
        const static int max_num_instrument = 50000; 
        // Market Data
        int num_chunks, chunk_ms;
        int chunk_i = 0;

        size_t offset = 0;
        size_t length = 0;
        std::vector<std::array<float, 4>> market_data;
        std::array<std::array<int, 2>, max_num_instrument> market_data_ptr;
        std::vector<std::array<int, 2>> book_update_ptr;
        int next_update_time;
        int next_update_idx;

        std::array< MarketEvent, max_num_instrument> orderbook;

        // Request
        DataRequest dummy_dr;
        std::unordered_map<std::string, std::string> network_channel = {
            {"Broker" , "BD"}
        };

        std::map<std::string, DataRequest> data_request_map = {
            {"Broker" , dummy_dr},
            {"Trader", dummy_dr},
            {"Strategist", dummy_dr},
            {"Risk_Manager", dummy_dr}
        };

        
    public:
        DataHandler();
        void get_chunk_info();
        void get_market_data_offset();
        void get_market_data_ptr();
        void get_book_update_ptr();
        void get_market_data();
        void update_data();
        void update_orderbook();
        void receive_request();
        void handle_request();
        void start();
};
#endif