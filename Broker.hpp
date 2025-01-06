#include <vector>
#include <string>
#include <iostream>
#include "Events.hpp"
#include <map>
#include <mpi.h>
#include <fstream>
#include "Helper.hpp"
#include "Participant.hpp"

#ifndef BROKER_H
#define BROKER_H

class Broker:public Base_Participant{
    private:


        // Market Data Path (probably need to move to DataHandler)
        std::string instrument_name_path;
        std::vector<std::string> instrument_name_list;

        std::unordered_map <int, std::vector<OrderEvent>> order_event_queue; // Key represents time
        std::unordered_map <std::string, MarketEvent> order_book;

        // Communication - Data Handler
        int world_size = 0;
        int num_dh;
        int self_update = 0;
        // Request
        std::vector<int> req_list = {};

        // Margin
        double stock_borrowing_cost = 0.3f;
        double margin_borrowing_cost = 0.05f;

    public:
        Broker();
        void update_tradable_instrument();
        void get_data_len_disp(){};
        void generate_orderbook_request();
        void request_orderbook();
        void receive_request();
        void handle_request();
        void start();
};
#endif