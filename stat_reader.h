#pragma once

#include <iosfwd>
#include <string_view>
#include <iostream>
#include <fstream>
#include<optional>
#include "transport_catalogue.h"

namespace transport_catalogue {

namespace output {


void ParseAndPrintStat(const transport_catalogue::main::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

namespace detail {
inline void StatReader(main::TransportCatalogue& catalogue,std::istream& input,std::ostream& output){
    int stat_request_count;
    std::string line;
    std::getline(input, line);
    stat_request_count = stoi(line);
    for (int i = 0; i < stat_request_count; ++i) {
        std::getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

} // end namespace detail

}

}
