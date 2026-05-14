#pragma once
#include <unordered_map>
#include <deque>
#include <string_view>
#include<optional>
#include <vector>
#include <set>
#include "geo.h"
#include "domain.h"

using namespace transport_catalogue::domain;
namespace transport_catalogue {
namespace main {

struct BusStat{
    int stops_on_route = 0;
    int uniq_stops_on_route = 0;
    double route_length = 0;
    double curvature = 0;
};
class RouteItem{
public:
     RouteItem(){};
    std::string type ="";
    double time = 0;
    ~RouteItem(){};

};
class RouteItemWait: public RouteItem{
public:
    std::string type = "Wait";
    std::string stop_name = "";
    double time = 0;

};
class RouteItemBus: public RouteItem{
public:
    std::string type = "Bus";
    std::string bus = "";
    int span_count = 0;
    double time = 0;

};

struct RouteStat{
    int total_time = 0;
    std::vector<RouteItem> route_stat = {};

};


class TransportCatalogue {
public:
    void AddBusRoute(Bus bus);
    void AddBusStop(Stop stop);
    void AddStopDistance(std::string_view stop_name1,std::string_view stop_name2, double distance);
    const Bus* FindRouteByName(std::string_view name) const;
    const Stop* FindBusStopByName(std::string_view name) const;
    /* не возвращает константную ссылку, так как функция возвращает копию локальной переменной */
    const std::set<std::string_view> GetBusesByStopName(std::string_view name) const;
    const std::deque<Bus>& GetAllBuses() const;
    double GetDistance(std::pair<Stop *, Stop *>);
    std::optional<double> RealDistanceCalculator(const Stop*,const Stop*) const;
    std::optional<main::BusStat> GetBusStat(const std::string_view name) const;
    bool GetRouteStat(std::vector<RouteStat>& route_stat,const std::pair<std::string,std::string> stops) const;
    std::optional<std::set<std::string_view>> GetStopInfo(std::string_view name) const;


private:
    struct PairHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& stop)  const {
            std::size_t h1 = std::hash<const Stop*>{}(stop.first);
            std::size_t h2 = std::hash<const Stop*>{}(stop.second);
            return h1 + 37 * h2;
        }
    };

    std::unordered_map<std::string_view,Stop *> stopname_to_stop_;
    std::unordered_map<std::string_view,Bus *> busname_to_bus_;
    std::unordered_map<std::string_view,std::set<std::string_view>> stopname_to_bus_list_;
    std::optional<std::set<std::string_view>> stop_info;
    std::unordered_map<std::pair<const Stop*,const Stop*>,double,PairHasher> real_distance_;
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;


};

} // end namespace main
} //end namespace
