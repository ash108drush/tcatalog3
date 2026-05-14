#include "stat_reader.h"
#include "transport_catalogue.h"
#include <iostream>
#include <iomanip>

using namespace transport_catalogue::main;

namespace transport_catalogue {

namespace output {


void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto colon_pos = request.find(':');
    auto space_pos = request.find(' ');
    std::string_view name = request.substr(space_pos + 1, colon_pos - space_pos +1);
    std::string_view command = request.substr(0, space_pos);
    output << std::setprecision(6);

    if(command == "Bus"){
        std::optional<BusInfo> opt_bus_info = transport_catalogue.GetBusInfo(name);
        if(opt_bus_info != std::nullopt){
            BusInfo bus_info = opt_bus_info.value();
            output <<  request << ": " << bus_info.stops_on_route << " stops on route, "
                   << bus_info.uniq_stops_on_route << " unique stops, "
                   << bus_info.route_length << " route length, "
                   << bus_info.curvature << " curvature" << std::endl;
            }else{
            output <<  request << ": not found" << std::endl;
        }
    }

    if(command == "Stop"){
        std::optional<std::set<std::string_view>> stop_info = transport_catalogue.GetStopInfo(name);
        if(stop_info != std::nullopt){
            if(stop_info.value().empty()){
                output <<  request << ": no buses" << std::endl;
            }else{
                output <<  request << ": buses ";
                size_t j = 0;
                const size_t max = stop_info.value().size();
                for(std::string_view stop_name : stop_info.value()){
                    output << stop_name;
                    ++j;
                    if(j != max){
                        output << ' ';
                    }
                }
                output << std::endl;
            }
        }else{
            output <<  request << ": not found" << std::endl;
        }
    }

}


}
} // end namespace
