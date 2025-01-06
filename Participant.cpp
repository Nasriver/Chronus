#include "Participant.hpp"

Base_Participant::Base_Participant(){};
void Base_Participant::update_backtest_settings(const std::string _start_date, const std::string _end_date, const int _start_time, const int _end_time, const std::string _market_data_folder_path){
    start_date = _start_date;
    end_date = _end_date;
    start_time = _start_time;
    end_time = _end_time;
    market_data_folder_path = _market_data_folder_path;
    cur_date = start_date;
    market_data_path = market_data_folder_path + cur_date + ".h5";
}

void Base_Participant::update_mpi_settings(const std::unordered_map <std::string, MPI_Group> _mpi_grps, std::unordered_map <std::string, MPI_Comm> _mpi_comms, const std::unordered_map<std::string, MPI_Datatype> _mpi_custom_type_map){
    mpi_grps = _mpi_grps;
    mpi_comms = _mpi_comms;
    mpi_custom_type_map = _mpi_custom_type_map;
}
                
void Base_Participant::update_cur_date(){
    if (cur_time == end_time){
        if (cur_date == end_date){
            status = "Closed";
        }
        else{
            while (1){
                cur_date = time_increment_one_days(cur_date);
                market_data_path = market_data_folder_path + cur_date + ".h5";

                if (std::filesystem::exists(market_data_path)){
                    cur_time = 0;
                    break;
                }
                else if (cur_date == end_date){
                    cur_time = 0;
                    break;
                }
            }
        }  
    }
}

void Base_Participant::update_cur_time(){
    cur_time += 1;
}

Base_Participant::~Base_Participant(){};