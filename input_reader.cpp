#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <array>
#include<string>
#include <vector>
#include <cstdlib>
using namespace std;
using namespace transport_catalogue::reader;
using namespace transport_catalogue::main;

namespace transport_catalogue {

namespace reader {

namespace detail {


/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

} //end namespace detail

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
void ParseCoordinates(std::string_view str, geo::Coordinates& coord, int &c) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        coord= {nan, nan};
        return;
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    size_t comma2 = str.find(',', not_space2);
    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double  lng=0;
    if (comma2 != str.npos) {
        lng = std::stod(std::string(str.substr(not_space2,comma2-not_space2)));
        c = comma2;
    }else{
        lng = std::stod(std::string(str.substr(not_space2)));

    }
    coord = {lat, lng};

}
struct DistanceInfo {
    std::string stop_name;
    double distance;
};

std::vector<DistanceInfo> ParseRealDistance(std::string str) {
    std::vector<DistanceInfo> distances;
    std::vector<std::string> str_vec;

    auto comma = str.find(',');
    if (comma == str.npos) {
        str_vec.push_back(str);
    } else {
        size_t begin = 0;
        do {
            str_vec.push_back(str.substr(begin, comma - begin));
            begin = comma + 2;
            comma = str.find(',', begin);
        } while (comma != str.npos);
        str_vec.push_back(str.substr(begin));
    }

    for (const auto& s : str_vec) {
        auto splitter = s.find("m to ");
        distances.emplace_back(DistanceInfo{s.substr(splitter + 5), std::stod(s.substr(0, splitter))});
    }

    return distances;

}


/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return detail::Split(route, '>');
    }

    auto stops = detail::Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

reader::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue)  {

    auto iterator_stop_block = std::partition(commands_.begin(),commands_.end(),[](const CommandDescription& el){
        return   (el.command == "Stop");
    });

    std::vector<std::pair<std::string,std::string>> stop_names;
    std::vector<double> stop_distances;

    for( auto iter = commands_.begin(); iter != iterator_stop_block; ++iter){
            std::string name = iter->id;
            geo::Coordinates coord = {};
            int comma=0;
            ParseCoordinates(iter->description,coord,comma);
            if(comma != 0){
                std::vector<std::string> vec ={};
                string distance_part = std::string(iter->description.substr(comma+2));                
                std::vector<DistanceInfo> distances = ParseRealDistance(distance_part);
                for(auto& stop : distances){
                   // catalogue.AddStopDistance(name, st.first, st.second);
                    stop_names.push_back({name,std::move(stop.stop_name)});
                    stop_distances.push_back(std::move(stop.distance));
                }

            }
            catalogue.AddBusStop(Stop{name,coord});
    }

    for(size_t i = 0;i < stop_names.size();i++){
        catalogue.AddStopDistance(stop_names[i].first, stop_names[i].second, stop_distances[i]);
    }

    for(auto iter = iterator_stop_block; iter != commands_.end(); ++iter){
                std::string name = iter->id;
                std::vector<std::string_view> route_parsed = ParseRoute(iter->description);
                std::vector<std::string_view> route;
                 for(auto value:route_parsed){
                    route.push_back(catalogue.FindBusStopByName(value)->name);
                }
                catalogue.AddBusRoute(Bus{name,route});
    }
}


}
} //end namespace
