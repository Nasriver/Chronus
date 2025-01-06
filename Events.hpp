#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <mpi.h>

#ifndef EVENTS_H
#define EVENTS_H

struct BaseEvent{
    public:
        int instrument_id = -1;
        int ms_of_day = -1;
        BaseEvent();
        virtual ~BaseEvent();
};

struct MarketEvent{
    public:
        int instrument_id = -1;
        int ms_of_day = -1;
        float bid;
        int bid_size;
        float ask;
        int ask_size;
        MarketEvent(int _instrument_id = -1, int _ms_of_day = -1, float _bid = -1, int _bid_size = -1, float _ask = -1, int _ask_size = -1);
        ~MarketEvent();
};

struct OrderEvent: public BaseEvent {
    public:
        float price;
        int size; 
        int direction; // Long, Short
        int order_type; // Market, Limit, Stop, Stop Limit
        int valid_cond; // FillorKill, GoodUntilCancelled, Day
        OrderEvent(int _instrument_id, int _ms_of_day, float _price, int _size, int _direction, int _order_type, int _valid_cond);
        ~OrderEvent();
};

struct FilledEvent: public BaseEvent {
    public:
        float price;
        int filled_size;
        float commission;
        FilledEvent(float _price, int _filled_size, float _commission);
        ~FilledEvent();
};

struct DataRequest{
    public:
        int instrument_idxs[200]; 
        DataRequest(std::vector<int> _instrument_idxs = {});
        ~DataRequest();
};

struct BrokerMessage{
    public:
        std::vector<MarketEvent> market_event;
        std::vector<FilledEvent> filled_event;
        BrokerMessage(std::vector<MarketEvent> _market_event, std::vector<FilledEvent> _filled_event);
        ~BrokerMessage();
};

MPI_Datatype create_market_event_mpi();
MPI_Datatype create_data_request_event_mpi();
std::unordered_map<std::string, MPI_Datatype> create_all_event_mpi();

#endif // EVENTS_H