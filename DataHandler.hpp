#include <vector>
#include <string>
#include <HighFive/HighFive.hpp>
#include "Events.hpp"
#include <mpi.h>
#include <map>
#include <HighFive/HighFive.hpp>
#include <format>
#include "helper.hpp"
#include "robin_hood.h"

#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H
class DataHandler{
    private:
        // MPI related:
        std::map <std::string, MPI_Group> mpi_grps;
        std::map <std::string, MPI_Comm> mpi_comms;
        std::map<std::string, MPI_Datatype> mpi_custom_type_map;

        int datahandler_rank;
        int datahandler_size;
        // Time
        std::string cur_date;
        int cur_time;
        
        // Market Data
        std::string market_data_folder_path;
        std::string market_data_path;
        
        
        std::vector<std::vector<float>> market_data;
        const static int max_num_instrument = 50000;
        const static int max_trading_ms = 24300000;
        robin_hood::unordered_map<int, std::vector<int>> market_data_idx_map; // temporary object
        int* market_data_idx_arr = NULL;
        std::vector<int> market_data_idx_ptr;
        int cur_idx_map[max_num_instrument] = {-999};
        robin_hood::unordered_map<int, std::vector<int>> book_update_map; // temporary object
        std::vector<int> book_update_arr;
        std::vector<int> book_update_ptr;
        std::vector<int> book_update_len;

        

        MarketEvent orderbook[max_num_instrument];

        int section_time;
        size_t offset = 0;
        size_t length = 0;

        // Request
        DataRequest dummy_dr;
        // need to add more relationship after including new character
        std::map<std::string, std::string> network_channel = {
            {"Broker" , "BD"}
        };

        std::map<std::string, DataRequest> data_request_map = {
            {"Broker" , dummy_dr},
            {"Trader", dummy_dr},
            {"Strategist", dummy_dr},
            {"Risk_Manager", dummy_dr}
        };

        
    public:
        DataHandler(std::string _cur_date, int _cur_time, const std::string _market_data_folder_path, const int _section_time, 
                    const std::map <std::string, MPI_Group> _mpi_grps, std::map <std::string, MPI_Comm> _mpi_comms, const std::map<std::string, MPI_Datatype> _mpi_custom_type_map);
        void update_cur_time(int _cur_time);
        void update_cur_date(std::string _cur_date);
        void get_instrument_dataset_offset(const std::string& market_dataset_name);
        void get_instrument_dataset(const int start, const int end);
        void update_orderbook();
        void receive_request();
        void handle_request();

};
#endif