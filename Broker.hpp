#include <vector>
#include <string>
#include <iostream>
#include "Events.hpp"
#include <map>
#include <mpi.h>
#include <filesystem>
#include <fstream>
#include "helper.hpp"
#ifndef BROKER_H
#define BROKER_H

class Broker{
    private:
        // MPI related:
        std::map <std::string, MPI_Group> mpi_grps;
        std::map <std::string, MPI_Comm> mpi_comms;
        std::map<std::string, MPI_Datatype> mpi_custom_type_map;
        const static int n_core = 5;

        // Broker Name
        std::string name;
        int cur_time;
        std::string cur_date;

        // Market Data Path (probably need to move to DataHandler)
        std::string market_data_folder_path;
        std::string instrument_name_path;
        std::vector<std::string> instrument_name_list;

        std::unordered_map <int, std::vector<OrderEvent>> order_event_queue; // Key represents time
        std::unordered_map <std::string, MarketEvent> order_book;
        std::string status = "Open";

        // Margin
        double stock_borrowing_cost = 0.3f;
        double margin_borrowing_cost = 0.05f;

    public:
        Broker(const std::string& _name, std::string _cur_date, int _cur_time, const std::string _market_data_folder_path, 
                const std::map <std::string, MPI_Group> _mpi_grps, std::map <std::string, MPI_Comm> _mpi_comms, const std::map<std::string, MPI_Datatype> _mpi_custom_type_map);
        void get_name();
        void update_cur_time(int _cur_time);
        void update_cur_date(std::string _cur_date);
        void update_tradable_instrument();
        void request_orderbook();
        void send_message();
};
#endif