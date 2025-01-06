#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <mpi.h>
#include "Events.hpp"
#include "DataHandler.hpp"
#include "Broker.hpp"
#include "Helper.hpp"
#include <memory>

// Backtesting Setting
std::string market_data_folder_path = "/Users/alanriver/Desktop/Projects/Backtester_C++/MarketData/";
std::string start_date = "20240801";
std::string end_date = "20240801";

int start_time = 0;
int end_time = 24300000;

// Backtest Engine
int main(int argc, char** argv){
    // Character Setting
    // MPI Setting
    int world_size, world_rank;
    MPI_Group world_grp;
    
    const std::unordered_map <std::string, std::vector<int>> ranks = {
        {"Broker", {0}}, 
        {"Data_Handler", {1, 2, 3, 4, 5}},
        {"Trader", {6}},
        {"Strategist", {7, 8}},
        {"Risk_Manager", {9}},
        {"BD", {0, 1, 2, 3, 4, 5}}, // For communication between Broker and DataHandler (Data Transfer)
        {"TD", {1, 2, 3, 4, 5, 6}}, // For communication between Trader and DataHandler (Data Transfer)
        {"SD", {1, 2, 3, 4, 5, 7, 8}}, // For communication between Strategist and DataHandler (Data Transfer)
        {"RD", {1, 2, 3, 4, 5, 9}}, // For communication between Risk Manager and DataHandler (Data Transfer)
        {"BT", {0, 6}}, // For communication between Broker and Trader (Order Execution)
        {"STR", {6, 7, 8, 9}}, // For communication between Trader, Strategist and Risk Manager (For new positions assessment)
        {"TR", {6, 9}}, // For communication between Trader and Risk Manager (For existing positions assessment)
    };

    std::unordered_map <std::string, MPI_Group> mpi_grps;
    std::unordered_map <std::string, MPI_Comm> mpi_comms;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_group(MPI_COMM_WORLD, &world_grp);
    mpi_grps["All"] = world_grp;
    mpi_comms["All"] = MPI_COMM_WORLD;

    // Initiate custom datatype for MPI
    std::unordered_map<std::string, MPI_Datatype> mpi_custom_type_map = create_all_event_mpi();

    for (auto [character, ranks] : ranks){
        int* tmp_rank = &ranks[0];
        MPI_Group tmp_grp;
        MPI_Comm tmp_comm;

        MPI_Group_incl(world_grp, ranks.size() , tmp_rank, &tmp_grp);
        MPI_Comm_create_group(MPI_COMM_WORLD, tmp_grp, 0, &tmp_comm);

        mpi_grps[character] = tmp_grp;
        mpi_comms[character] = tmp_comm;
    };
    // Network Latency (in ms)
    // Broker has the highest priority of recieving market updates
    // Assume Same Latency for other channel 
    
    // WIP

    // Initiate  Participants for each rank
    switch (world_rank){
    case 0:{
        Broker broker;
        broker.update_backtest_settings(start_date, end_date, start_time, end_time, market_data_folder_path);
        broker.update_mpi_settings(mpi_grps, mpi_comms, mpi_custom_type_map);
        broker.start();
        break;
    }
       
    case 1: case 2: case 3: case 4: case 5:{
        DataHandler datahandler;
        datahandler.update_backtest_settings(start_date, end_date, start_time, end_time, market_data_folder_path);
        datahandler.update_mpi_settings(mpi_grps, mpi_comms, mpi_custom_type_map);
        datahandler.start();
        break;
    }

    default:
        break;
    }

    MPI_Finalize();
    return 0;
};



