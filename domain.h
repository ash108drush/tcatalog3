#pragma once
#include <unordered_map>
#include <deque>
#include <string_view>
#include <vector>
#include <string>
#include "geo.h"

namespace transport_catalogue {
namespace domain {
struct Bus {
    std::string name;
    std::vector<std::string_view> stops ={};
    bool is_roundtrip;
};

struct Stop{
    std::string name;
    geo::Coordinates coordinates;
    std::unordered_map<std::string_view,double> road_distances = {};
};

struct BusRoute {
    std::vector<const Stop *> stops ={};
    bool is_roundtrip;
    const Stop * last_stop =nullptr;
};

} //end namespace domain
} //end namespace transport_catalogue
