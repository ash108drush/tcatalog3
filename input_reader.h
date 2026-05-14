#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

namespace reader {

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }


    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
     */
    void ParseLine(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(main::TransportCatalogue& catalogue);


private:
    std::vector<CommandDescription> commands_;
};

namespace detail {

inline void ReadInput(main::TransportCatalogue& catalogue,std::istream& input){
    int base_request_count;
    std::string line;
    std::getline(input, line);
    base_request_count = stoi(line);
    InputReader reader;
    for (int i = 0; i < base_request_count; ++i) {
        std::getline(input, line);
        reader.ParseLine(line);
    }
    reader.ApplyCommands(catalogue);
}

} //end namespace detail

}
} //end namespace
