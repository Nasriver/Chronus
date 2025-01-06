#include "DataHandler.hpp"

DataHandler::DataHandler(){}

void DataHandler::get_chunk_info(){
    HighFive::FileAccessProps fapl;
    HighFive::DataTransferProps xfer_props;
    fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
    fapl.add(HighFive::MPIOCollectiveMetadata{});
    xfer_props.add(HighFive::UseCollectiveIO{});

    std::string grp_name = "ChunkDescription";
    std::string dataset_name = "ChunkInfo";
    
    HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
    auto file_group = file.getGroup(grp_name);
    HighFive::DataSet dataset = file_group.getDataSet(dataset_name);
    
    std::array<int, 2>  chunk_info;

    dataset.read(chunk_info, xfer_props);
    num_chunks = chunk_info[0];
    chunk_ms = chunk_info[1];
}

void DataHandler::get_market_data_offset(){
    HighFive::FileAccessProps fapl;
    HighFive::DataTransferProps xfer_props;
    fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
    fapl.add(HighFive::MPIOCollectiveMetadata{});
    xfer_props.add(HighFive::UseCollectiveIO{});
    
    std::string grp_name = "/ChunkDescription/";
    std::string dataset_name = "Chunk_"+std::to_string(chunk_i)+"_OffsetLength";
    
    HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
    auto file_group = file.getGroup(grp_name);
    HighFive::DataSet dataset = file_group.getDataSet(dataset_name);

    std::array<std::array<int, 2>, 1> offset_length;

    dataset.select({(size_t)datahandler_rank, 0}, {1, 2}).read(offset_length, xfer_props);
    offset = offset_length[0][0];
    length = offset_length[0][1];

}

void DataHandler::get_market_data_ptr(){
    HighFive::FileAccessProps fapl;
    HighFive::DataTransferProps xfer_props;
    fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
    fapl.add(HighFive::MPIOCollectiveMetadata{});
    xfer_props.add(HighFive::UseCollectiveIO{});

    std::string grp_name = "/ChunkDescription/";
    std::string dataset_name = "Chunk_"+std::to_string(chunk_i)+"_MarketDataPtr";

    HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
    auto file_group = file.getGroup(grp_name);
    HighFive::DataSet dataset = file_group.getDataSet(dataset_name);
    
    dataset.read(market_data_ptr, xfer_props);
}

void DataHandler::get_book_update_ptr(){
    HighFive::FileAccessProps fapl;
    HighFive::DataTransferProps xfer_props;
    fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
    fapl.add(HighFive::MPIOCollectiveMetadata{});
    xfer_props.add(HighFive::UseCollectiveIO{});
    
    std::string grp_name = "/ChunkDescription/";
    std::string dataset_name = "Chunk_"+std::to_string(chunk_i)+"_BookUpdatePtr";
    
    HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
    auto file_group = file.getGroup(grp_name);
    HighFive::DataSet dataset = file_group.getDataSet(dataset_name);
    
    book_update_ptr.reserve(length);
    dataset.select({offset , 0}, {length, 2}).read(book_update_ptr, xfer_props);
    next_update_time = book_update_ptr[0][0];
    next_update_idx = 0;
}

void DataHandler::get_market_data() {
    HighFive::FileAccessProps fapl;
    HighFive::DataTransferProps xfer_props;
    fapl.add(HighFive::MPIOFileAccess{mpi_comms["Data_Handler"], MPI_INFO_NULL});
    fapl.add(HighFive::MPIOCollectiveMetadata{});
    xfer_props.add(HighFive::UseCollectiveIO{});

    std::string grp_name = "/MarketData/";
    std::string dataset_name = "Chunk_"+std::to_string(chunk_i);

    HighFive::File file(market_data_path, HighFive::File::ReadOnly, fapl);
    auto file_group = file.getGroup(grp_name);
    HighFive::DataSet dataset = file_group.getDataSet(dataset_name);

    market_data.reserve(length);
    dataset.select({offset , 0}, {length, 4}).read(market_data, xfer_props);

};

void DataHandler::update_data(){
    if (cur_time % chunk_ms == 0){
        market_data.clear();

        Timer timer;
        timer.start();
        get_market_data_offset();
        get_market_data_ptr();
        get_book_update_ptr();
        get_market_data();
        chunk_i++;
        timer.stop();
        if (datahandler_rank == 0){
            timer.time_elapsed("s", "Loading Data");
        }
    }
}

void DataHandler::update_orderbook(){
    if (cur_time == next_update_time){
        int i = next_update_idx;
        int update_time;
        int instrument_idx, instrument_cur_ptr;
        while(1){
            update_time = book_update_ptr[i][0];
            instrument_idx = book_update_ptr[i][1];
            
            if (update_time == cur_time){
                market_data_ptr[instrument_idx][0]++;
                market_data_ptr[instrument_idx][1]--;
                instrument_cur_ptr = market_data_ptr[instrument_idx][0];
                orderbook[instrument_idx] = get_market_event(instrument_idx, update_time, market_data[instrument_cur_ptr]);

            }
            else{
                next_update_time = update_time;
                next_update_idx = i;
                break;
            }
            i++;
        }

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
            MarketEvent me_list[max_num_instrument];
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

void DataHandler::start(){
    
    
    MPI_Comm_rank(mpi_comms["Data_Handler"], &datahandler_rank);
    MPI_Comm_size(mpi_comms["Data_Handler"], &datahandler_size);
    get_chunk_info();
    
    while (status != "Closed"){
        update_cur_date();
        update_data();
        update_orderbook();
        receive_request();
        handle_request();
        update_cur_time();
    }
    
    
   
}