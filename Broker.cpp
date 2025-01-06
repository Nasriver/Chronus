#include "Broker.hpp"

// Constructor
Broker::Broker(){}
    
// Get tradable instrument name from the HDF5 file
void Broker::update_tradable_instrument(){
    instrument_name_path = market_data_folder_path + cur_date + "_Instruments.txt";
    std::ifstream reader(instrument_name_path);

    if (!reader.is_open()) {
        std::cerr << "Error opening the file!";
        return;
    }

    std::string instrument_name;

    while (getline(reader, instrument_name)){
        instrument_name_list.push_back(instrument_name);
    }
}

void Broker::generate_orderbook_request(){
    // Making some random request, but in reality this depends on client order and existing client positions
    if ((cur_time % (1 * 1000)) == 0 & cur_time != 0){
        req_list.push_back(-1);
    }

    else{
        for (int i=0; i<num_dh; i++){
            //req_list.push_back(0 + i);
            req_list.push_back(8386 + i);
        }
    }
}

void Broker::request_orderbook(){
    DataRequest dr(req_list);
    MPI_Datatype dr_type = mpi_custom_type_map["DataRequestEvent"];
    MPI_Bcast(&dr, 1, dr_type, 0, mpi_comms["BD"]);

    if (req_list.size() > 0){
        
        int* orderbook_len_arr = (int*) malloc(world_size * sizeof(int));
        int* disp = (int*) malloc(world_size * sizeof(int));
        int total_updates = 0;

        
        MPI_Gather(&self_update, 1, MPI_INT, 
                    orderbook_len_arr, 1, MPI_INT, 
                    0, mpi_comms["BD"]);

        for (int i = 0; i < world_size ; i++){
            if (i != 0){
                total_updates += orderbook_len_arr[i];
            }

            if (i < num_dh){
                disp[i+1] += disp[i] + orderbook_len_arr[i];
            }
        }

        MarketEvent* me_list = (MarketEvent*)malloc(total_updates * sizeof(MarketEvent));
    
        MPI_Datatype me_type = mpi_custom_type_map["MarketEvent"];
        

        MPI_Gatherv(NULL, 0, me_type, 
                    me_list, orderbook_len_arr, disp, me_type, 
                    0, mpi_comms["BD"]);
        
        for (int i = 0; i < total_updates; i++){
            //MarketEvent me = me_list[i];
            //std::cout << "Current Time: " << cur_time << " ";
            //std::cout << "Instrument ID: " << me.instrument_id;
            //std::cout<<" Time: " << me.ms_of_day;
            //std::cout << " Bid: " << me.bid << " Bid Size: " << me.bid_size;
            //std::cout << " Ask: " << me.ask << " Ask Size: " << me.ask_size << std::endl;
        }

        free(me_list);
        free(orderbook_len_arr);
        free(disp);

        req_list.clear();
    }
}

void Broker::receive_request(){} // WIP

void Broker::handle_request(){} // WIP

void Broker::start(){
    Timer timer;
    timer.start();
    MPI_Comm_size(mpi_comms["BD"], &world_size);
    num_dh = world_size - 1;
    
    while(status != "Closed"){
        if (cur_time % (10*60*1000) == 0){
            std::cout<<"Current Time: "<<cur_time<<std::endl;
        }
        update_cur_date();
        generate_orderbook_request();
        request_orderbook(); 
        update_cur_time();
    }

    timer.stop();
    timer.time_elapsed("s", "Full Backtest");
}
