#include "request_handler.h"
#include <cassert>


using namespace transport_catalogue;



void  RequestHandler::DrawBusRoute(std::unique_ptr<MapRenderer>&& map_renderer_ptr) const {
    const std::deque<Bus> all_buses_deq = db_.GetAllBuses();
    std::map<std::string_view,BusRoute> bus_stops;
    const Stop * last_stop = nullptr;
    for(const auto &bus: all_buses_deq){
        std::vector<const Stop *> stops;
        for(auto iter = bus.stops.begin(); iter != bus.stops.end(); ++iter){
            const Stop * stop = db_.FindBusStopByName(*iter);
            if(stop != nullptr){
                stops.push_back(stop);
            }
            last_stop = stop;
        }

        if(!bus.is_roundtrip){
            for(auto iter = bus.stops.rbegin()+1; iter != bus.stops.rend();++iter){
                const Stop * stop = db_.FindBusStopByName(*iter);
                if(stop != nullptr){
                    stops.push_back(stop);
                }
            }
        }

        if(stops.size() >0){
            bus_stops.insert({bus.name,{stops,bus.is_roundtrip,last_stop}});

        }

    }

   map_renderer_ptr->RenderMap(bus_stops);

}
