#include <filesystem>
#include "Helper.hpp"

#ifndef PARTICIPANT_H
#define PARTICIPANT_H

class Base_Participant{
    public:
        // Time related:
        std::string start_date, end_date, cur_date;
        int start_time, end_time, cur_time;

        // MPI related:
        std::unordered_map <std::string, MPI_Group> mpi_grps;
        std::unordered_map <std::string, MPI_Comm> mpi_comms;
        std::unordered_map<std::string, MPI_Datatype> mpi_custom_type_map;

        // Info
        std::string market_data_folder_path;
        std::string market_data_path;
        std::string status = "Open";

        // Functions
        Base_Participant();
        void update_cur_date();
        void update_cur_time();
        void update_backtest_settings(const std::string _start_date, const std::string _end_date, const int _start_time, const int _end_time, const std::string _market_data_folder_path);
        void update_mpi_settings(const std::unordered_map <std::string, MPI_Group> _mpi_grps, std::unordered_map <std::string, MPI_Comm> _mpi_comms, const std::unordered_map<std::string, MPI_Datatype> _mpi_custom_type_map);
        ~Base_Participant();

};
#endif