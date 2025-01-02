#include "DataHandler.hpp"

DataHandler::DataHandler(std::string _cur_date, int _cur_time, const std::string _market_data_folder_path, const int _section_time, const std::map <std::string, MPI_Group> _mpi_grps, std::map <std::string, MPI_Comm> _mpi_comms, const std::map<std::string, MPI_Datatype> _mpi_custom_type_map){
    cur_date = _cur_date;
    cur_time = _cur_time;
    market_data_folder_path = _market_data_folder_path;
    section_time = _section_time;
    mpi_grps = _mpi_grps;
    mpi_comms = _mpi_comms;
    mpi_custom_type_map = _mpi_custom_type_map;
    MPI_Comm_rank(mpi_comms["Data_Handler"], &datahandler_rank);
    MPI_Comm_size(mpi_comms["Data_Handler"], &datahandler_size);

}

void DataHandler::update_cur_time(int _cur_time){
    // Update cur time
    cur_time = _cur_time;
    if (cur_time%section_time == 0){
        // release memory
        market_data.clear();
        market_data_idx_map.clear();
        book_update_map.clear();
        free(market_data_idx_arr);
        market_data_idx_ptr.clear();
        book_update_arr.clear();
        book_update_ptr.clear();
        book_update_len.clear();
       

        get_instrument_dataset(cur_time+34200000, cur_time+34200000+section_time);
    }
    update_orderbook();
}

void DataHandler::update_cur_date(std::string _cur_date){
    cur_date = _cur_date;
    market_data_path = market_data_folder_path + cur_date + "_Data.h5";
    MarketEvent dummy_me;
    // Initialize orderbook map
    for (int i = 0; i<max_num_instrument; i++){
        orderbook[i] = dummy_me;
    }
}

void DataHandler::get_instrument_dataset_offset(const std::string& market_dataset_name){

    HighFive::FileAccessProps fapl;
    fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
    fapl.add(HighFive::MPIOCollectiveMetadata{});
    HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
    auto xfer_props = HighFive::DataTransferProps{};
    xfer_props.add(HighFive::UseCollectiveIO{});

    std::string dataset_name = "section_details_"+market_dataset_name;
    
    std::vector<std::vector<int>> num_rows_sect;

    HighFive::DataSet dataset = file.getDataSet(dataset_name);
    dataset.read(num_rows_sect, xfer_props);

    offset = num_rows_sect[datahandler_rank][0];
    length = num_rows_sect[datahandler_rank][1];

}
void DataHandler::get_instrument_dataset(const int start, const int end) {
    // now use c++ original map first, later replace it with boost concurrent map
    // only load the data
    Timer timer;
    timer.start();
    try 
    {
        // Collective IO Implementation
        HighFive::FileAccessProps fapl;
        fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
        fapl.add(HighFive::MPIOCollectiveMetadata{});
        HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
        auto xfer_props = HighFive::DataTransferProps{};
        xfer_props.add(HighFive::UseCollectiveIO{});
        
        std::string dataset_name = "s" + std::to_string(start) + "_e" + std::to_string(std::min(end-1, 58500000));
        HighFive::DataSet dataset = file.getDataSet(dataset_name);
        
        // Adjusting nrow for last rank in DataHandler subworld.
        get_instrument_dataset_offset(dataset_name);
        market_data.reserve(length);
        dataset.select({offset , 0}, {length, 6}).read(market_data, xfer_props);

        timer.stop();
        timer.time_elapsed("s", "Loading Data");
        timer.start();
        // Prepare market data update queue object
        for (int i = 0; i < length; i++){
            int instrument_idx = market_data[i][5];
            int t = market_data[i][0];
            if (market_data_idx_map.find(instrument_idx) ==  market_data_idx_map.end()){
                market_data_idx_map[instrument_idx].reserve(50000);
            }
            market_data_idx_map[instrument_idx].push_back(i);
            book_update_map[t].push_back(instrument_idx);
        }

        market_data_idx_arr = (int*) malloc(length * sizeof(int));
        market_data_idx_ptr.reserve(max_num_instrument);
        book_update_arr.reserve(length);
        book_update_ptr.reserve(max_trading_ms);
        book_update_len.reserve(max_trading_ms);
        // Initialize pointer for market_data_idx
        int j = 0;
        for (auto& [instrument_idx, idxs] : market_data_idx_map){
            market_data_idx_ptr[instrument_idx] = j - 1; // for initial increment
            for (auto& idx : idxs){
                market_data_idx_arr[j] = idx;
                j++;
            }
            std::vector<int>().swap(idxs);
        }

        j = 0;
        for (auto& [t, instrument_idxs] : book_update_map){
            book_update_ptr[t] = j;
            book_update_len[t] = instrument_idxs.size();
            for (auto& instrument_idx : instrument_idxs){
                book_update_arr[j] = instrument_idx;
                j++;
            }
            std::vector<int>().swap(instrument_idxs);
        }

        timer.stop();

        //for (int i=0; i<max_num_instrument; i++){
            //if (market_data_idx_map.find(i) != market_data_idx_map.end()){
                //cur_idx_map[i] = -1;
            //}
        //}
        
        timer.time_elapsed("s", "Updating market data index map and current idx map");
    }
    
    catch (const HighFive::Exception& e) {
        std::cerr << "Error reading HDF5 file: " << e.what() << std::endl;
        MPI_Abort(mpi_comms["Data_Handler"], 1);
    }
    timer.stop();
    //timer.time_elapsed("s", "Updating Book Update Time");
};
void DataHandler::update_orderbook(){
    int start = book_update_ptr[cur_time];
    int end = start + book_update_len[cur_time];
    // Array Implementation:
    for (int i = start; i<end; i++){
        int instrument_idx = book_update_arr[i];
        market_data_idx_ptr[instrument_idx]++;
        int cur_idx = market_data_idx_ptr[instrument_idx];
        int cur_market_data_idx = market_data_idx_arr[cur_idx];
        orderbook[instrument_idx] = get_market_event(market_data[cur_market_data_idx]);
    }
    
}

void DataHandler::receive_request(){

    MPI_Datatype dr_type = mpi_custom_type_map["DataRequestEvent"];
    DataRequest dr;
    for (auto& [character, channel] : network_channel){
        MPI_Bcast(&dr, 1, dr_type, 0, mpi_comms[channel]);
        data_request_map[character] = dr;
    }
}

void DataHandler::handle_request(){

    MPI_Datatype me_type = mpi_custom_type_map["MarketEvent"];
    // Handling Data Request
    for (auto& [character, dr] : data_request_map){
        // Full Orderbook Snapshot
        if (dr.instrument_idxs[0] == -1){
            MarketEvent* me_list;
            me_list = (MarketEvent*) malloc(max_num_instrument * sizeof(MarketEvent));
            int n = 0;

            for (int instrument_idx = 0; instrument_idx<max_num_instrument; instrument_idx++){
                if (orderbook[instrument_idx].instrument_id != -1){
                    me_list[n] = orderbook[instrument_idx];
                    n++;
                }
            }
            MPI_Gather(&n, 1, MPI_INT, 
                        NULL, 0, 0, 
                        0, mpi_comms[network_channel[character]]);
                
            MPI_Gatherv(me_list, n, me_type, 
                        NULL, NULL , NULL,  me_type, 
                        0, mpi_comms[network_channel[character]]);
            
            free(me_list);
            
        }

        // Selected Orderbook Snapshot
        else if (dr.instrument_idxs[0] != -999){
            std::vector<int> task_list;

            for (auto& instrument_idx : dr.instrument_idxs){
                if (instrument_idx != -999){
                    if (orderbook[instrument_idx].instrument_id != -1){
                        task_list.push_back(instrument_idx);
                    }
                }
                else{
                    break;
                }
            }

            int n = task_list.size();
            MarketEvent* me_list;
            me_list = (MarketEvent*) malloc(n * sizeof(MarketEvent));
            for (int i = 0; i < n; i++){
                me_list[i] = orderbook[task_list[i]];
                //std::cout << "Instrument ID: " << me_list[i].instrument_id;
                //std::cout<<" Time: " << me_list[i].ms_of_day;
                //std::cout << " Bid: " << me_list[i].bid << " Bid Size: " << me_list[i].bid_size;
                //std::cout << " Ask: " << me_list[i].ask << " Ask Size: " << me_list[i].ask_size << std::endl;
            }

            MPI_Gather(&n, 1, MPI_INT, 
                        NULL, 1, MPI_INT, 
                        0, mpi_comms[network_channel[character]]);
            
            if (n > 0){
                MPI_Gatherv(me_list, n, me_type, 
                            NULL, NULL , NULL,  me_type, 
                            0, mpi_comms[network_channel[character]]);
            }
            else{
                MPI_Gatherv(NULL, 0, me_type, 
                            NULL, NULL , NULL,  me_type, 
                            0, mpi_comms[network_channel[character]]);                          
            }


            free(me_list);
        }
    }

}