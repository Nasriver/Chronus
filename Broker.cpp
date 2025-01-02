#include "Broker.hpp"

// Constructor
Broker::Broker(const std::string& _name, std::string _cur_date, int _cur_time, const std::string _market_data_folder_path, 
                const std::map <std::string, MPI_Group> _mpi_grps, std::map <std::string, MPI_Comm> _mpi_comms, const std::map<std::string, MPI_Datatype> _mpi_custom_type_map){
    name = _name;
    cur_date = _cur_date;
    cur_time = _cur_time;
    market_data_folder_path = _market_data_folder_path;
    mpi_grps = _mpi_grps;
    mpi_comms = _mpi_comms;
    mpi_custom_type_map = _mpi_custom_type_map;

}
        
void Broker::get_name() {
    std::cout<<"Broker Name:" + name <<std::endl;
}

void Broker::update_cur_time(int _cur_time){
    cur_time = _cur_time;
}

void Broker::update_cur_date(std::string _cur_date){
    cur_date = _cur_date;
    instrument_name_path = market_data_folder_path + cur_date + "_Instruments.txt";
    update_tradable_instrument();
}

// Get tradable instrument name from the HDF5 file
void Broker::update_tradable_instrument(){
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
void Broker::request_orderbook(){

    Timer timer;
    timer.start();
    // Reciever = Data Handler | Sender = Broker
    std::vector<int> req_list = {};
    int req_size = 0;
    // temp request
    if ((cur_time % (1 * 1000)) == 0 & cur_time != 0){
        //req_list.push_back(-1);

    }
    else{
        for (int i=0; i<1; i++){
            //req_list.push_back(1660 + i);
            req_list.push_back(13666 + i);
        }
    }

    DataRequest dr(req_list);
    MPI_Datatype dr_type = mpi_custom_type_map["DataRequestEvent"];   
    MPI_Bcast(&dr, 1, dr_type, 0, mpi_comms["BD"]);

    if (req_list.size() > 0){
        
        // need to adjust it with core size
        
        int orderbook_len_arr[n_core+1] = {0};
        int disp[n_core+1] = {0};

        int n = 0;
        timer.stop();
        //timer.time_elapsed("s", "1st Bcast");
        timer.start();

        MPI_Gather(&n, 1, MPI_INT, 
                    &orderbook_len_arr, 1, MPI_INT, 
                    0, mpi_comms["BD"]);
        
        timer.stop();
        //timer.time_elapsed("s", "1st Gatherv");
        timer.start();

        int i = 0;
        for (auto& len : orderbook_len_arr){
            if (i!=0){
                n += len;
            }
            if (i<n_core){
                disp[i+1] += disp[i] + len;
            }
            i++;
        }

        MarketEvent* me_list = (MarketEvent*)malloc(n * sizeof(MarketEvent));
    
        MPI_Datatype me_type = mpi_custom_type_map["MarketEvent"];

        MPI_Gatherv(NULL, 0, me_type, 
                    me_list, orderbook_len_arr, disp, me_type, 
                    0, mpi_comms["BD"]);
        for (int i = 0; i<n; i++){

            //MarketEvent me = me_list[i];
            //std::cout << "Current Time: " << cur_time << " ";
            //std::cout << "Instrument ID: " << me.instrument_id;
            //std::cout<<" Time: " << me.ms_of_day;
            //std::cout << " Bid: " << me.bid << " Bid Size: " << me.bid_size;
            //std::cout << " Ask: " << me.ask << " Ask Size: " << me.ask_size << std::endl;
        }
        free(me_list);
    }
    
    timer.stop();
    //timer.time_elapsed("s", "2nd Gatherv");
    
}

// Receive Request
// Handle Request
void Broker::send_message(){
    std::cout <<"WIP"<<std::endl;
}

