#include <iostream>
#include <map>
#include <vector>
#include "Events.hpp"
#include "DataHandler.hpp"
#include "helper.hpp"
#include <filesystem>
#include <string>
#include <chrono>
#include <thread>
#include <functional>
#include <future>
#include <mpi.h>

class Trader{
    private:
        string name;
        Mediator* world; // pointer for mundi object

    public:
        Trader(const string& _name, Mediator* _world){
            name = _name;
            world = _world;
        }

        int recieved_updates = 0;
        // Communication
        unordered_map <string, MarketEvent> order_book;

        void get_name() override{
            cout<<"Trader Name:" + name <<endl;
        }

        void handle_message() override{
            vector <BrokerMessage> msg_list;
            world->read_msg_from_broker(msg_list);
            int cur_time = world->get_cur_time();

            for (auto& msg : msg_list){
                for (auto me:msg.market_event){
                    //order_book.insert_or_assign(me.instrument, me);
                }
            }
            releaseVectorMemory_bm(msg_list);
        }

        void send_message() override{
            int cur_time = world->get_cur_time();
            if (cur_time % 1000 == 0){
                //cout<<"Trader:Now Sending Message to Broker: "<<cur_time<<endl;
                TraderMessage msg("FullBook_Snapshot", "None");
                world->send_msg_to_broker(msg);
            }
            //else if (cur_time % 5000 == 0){
                //world->send_msg_to_broker(TraderMessage("Instrument_Snapshot", "QQQ_20240621_610000"));
            //}
        };
};