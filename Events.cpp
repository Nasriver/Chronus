#include "Events.hpp"

BaseEvent::BaseEvent(){};
BaseEvent::~BaseEvent(){};
MarketEvent::MarketEvent(int _instrument_id, int _ms_of_day, float _bid, int _bid_size, float _ask, int _ask_size){
    instrument_id = _instrument_id;
    ms_of_day = _ms_of_day;
    bid = _bid;
    bid_size = _bid_size;
    ask = _ask;
    ask_size = _ask_size;
}
MarketEvent::~MarketEvent(){}

OrderEvent::OrderEvent(int _instrument_id, int _ms_of_day, float _price, int _size, int _direction, int _order_type, int _valid_cond){
    instrument_id = _instrument_id;
    ms_of_day = _ms_of_day;
    price = _price;
    size = _size;
    direction = _direction;
    order_type = _order_type;
    valid_cond = _valid_cond;
}
OrderEvent::~OrderEvent(){}

FilledEvent::FilledEvent(float _price, int _filled_size, float _commission){
    price = _price;
    filled_size = _filled_size;
    commission = _commission;
}

FilledEvent::~FilledEvent(){}

DataRequest::DataRequest(std::vector<int> _instrument_idxs){
    for (int i = 0; i<200; i++){
        if (i < _instrument_idxs.size()){
            instrument_idxs[i] = _instrument_idxs.at(i);
        }
        else{
            instrument_idxs[i] = -999;
            break;
        }
    }
}

DataRequest::~DataRequest(){}

BrokerMessage::BrokerMessage(std::vector<MarketEvent> _market_event, std::vector<FilledEvent> _filled_event){
    market_event =  _market_event;
    filled_event = _filled_event;
}

BrokerMessage::~BrokerMessage(){}

MPI_Datatype create_market_event_mpi(){
    MPI_Datatype me_type;
    MarketEvent me;

    int me_block_lengths[5] = {2, 1, 1, 1, 1};
    MPI_Datatype me_types[5] = {MPI_INT, MPI_FLOAT, MPI_INT, MPI_FLOAT, MPI_INT};
    
    MPI_Aint me_dp[5];
    MPI_Aint base_address;
    MPI_Get_address(&me, &base_address);
    MPI_Get_address(&me.instrument_id, &me_dp[0]);
    MPI_Get_address(&me.bid, &me_dp[1]);
    MPI_Get_address(&me.bid_size, &me_dp[2]);
    MPI_Get_address(&me.ask, &me_dp[3]);
    MPI_Get_address(&me.ask_size, &me_dp[4]);

    for (int i = 0; i < 5; i++){
        me_dp[i] = MPI_Aint_diff(me_dp[i], base_address);
    }
    
    MPI_Type_create_struct(5, me_block_lengths, me_dp, me_types, &me_type);
    MPI_Type_commit(&me_type);

    return me_type;
}

MPI_Datatype create_data_request_event_mpi(){
    MPI_Datatype dre_type;
    DataRequest dr;

    int dr_block_lengths[1] = {200};
    MPI_Datatype dr_types[1] = {MPI_INT};
    MPI_Aint dr_dp[1];

    MPI_Aint base_address;
    MPI_Get_address(&dr, &base_address);
    MPI_Get_address(&dr.instrument_idxs, &dr_dp[0]);

    for (int i = 0; i < 1; i++){
        dr_dp[i] = MPI_Aint_diff(dr_dp[i], base_address);
    }
    MPI_Type_create_struct(1, dr_block_lengths, dr_dp, dr_types, &dre_type);
    MPI_Type_commit(&dre_type);

    return dre_type;
}

std::map<std::string, MPI_Datatype>  create_all_event_mpi(){
    std::map<std::string, MPI_Datatype> custom_mpi_type_map;
    custom_mpi_type_map["MarketEvent"] = create_market_event_mpi();
    custom_mpi_type_map["DataRequestEvent"] = create_data_request_event_mpi();
 
    return custom_mpi_type_map;
}
