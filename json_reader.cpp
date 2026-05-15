#include "json_reader.h"
#include "json_builder.h"
#include "domain.h"
#include "map_renderer.h"
#include "geo.h"
#include <iomanip>

 namespace transport_catalogue {
using namespace std::literals;
 void JsonReader::MakeBaseRequests(json::Node nodes)const {
    if(nodes.IsArray()){
        for(const json::Node& node : nodes.AsArray()){
            if(node.IsDict()){
                const json::Dict dict = node.AsDict();
                auto iter = dict.find("type");
                if( iter != dict.end()){
                    if(iter->second.IsString()){
                        if(std::get<std::string>(iter->second.GetValue()) == "Stop"){
                            domain::Stop stop = ParseStop(dict);
                            db_.AddBusStop(stop);
                        }
                    }
                }
            }
        }
    }

    if(nodes.IsArray()){
        for(const json::Node& node : nodes.AsArray()){
            if(node.IsDict()){
                const json::Dict dict = node.AsDict();
                auto iter = dict.find("type");
                if( iter != dict.end()){
                    if(iter->second.IsString()){
                        if(std::get<std::string>(iter->second.GetValue()) == "Stop"){
                            auto iter_distance = dict.find("road_distances");
                            if( iter_distance != dict.end()){
                                if(iter_distance->second.IsDict()){
                                    auto iter_name = dict.find("name");
                                    std::string stop1_name = std::get<std::string>(iter_name->second.GetValue());
                                    const json::Dict distances = iter_distance->second.AsDict();
                                    for(const auto &[key,value]: distances){
                                        db_.AddStopDistance(stop1_name,key,value.AsDouble());
                                    }

                                }
                            }
                        }
                        if(std::get<std::string>(iter->second.GetValue()) == "Bus"){
                            domain::Bus bus = ParseBus(dict);
                            db_.AddBusRoute(bus);
                        }
                    }
                }
            }
        }
    }

}

void JsonReader::MakeStatRequests(json::Node nodes){
      if(nodes.IsArray()){
          for(const json::Node& node : nodes.AsArray()){
              if(node.IsDict()){
                  const json::Dict dict = node.AsDict();
                  auto iter = dict.find("type");
                  if(!iter->second.IsString()) return;
                  std::string type = std::get<std::string>(iter->second.GetValue());//iter->second.AsString();
                  if(type == "Map"){
                      MakeMapStatRequest(dict);
                  }
                  if(type == "Stop"){
                      MakeStopStatRequest(dict);
                  }
                  if(type == "Bus"){
                      MakeBusStatRequest(dict);
                  }
                  if(type == "Route"){
                      MakeRouteStatRequest(dict);
                  }
              }
          }
         PrintStatRequests();

      }
}

void JsonReader::MakeStopStatRequest(const json::Dict &dict){
    std::string name="";
    int id = 0;
    for(const auto &[key,value]: dict){
        if(key == "name"){
            name = std::get<std::string>(value.GetValue());
        }
        if(key == "id"){
            id = value.AsInt();
        }
    }

    std::optional<std::set<std::string_view>> stop_info = db_.GetStopInfo(name);
    if(stop_info.has_value()){
        Request new_request{id,stop_info.value()};
        stat_.push_back(new_request);
    }else{
        stat_.emplace_back(Request{id,nullptr});
    }
}

void JsonReader::MakeBusStatRequest(const json::Dict &dict){
    std::string name="";
    int id = 0;
    for(const auto &[key,value]: dict){
        if(key == "name"){
            name = std::get<std::string>(value.GetValue());
        }
        if(key == "id"){
            id = value.AsInt();
        }
    }
    std::optional<main::BusStat> bus_stat = db_.GetBusStat(name);
    if(bus_stat.has_value()){
        Request new_request{id,bus_stat.value()};
        stat_.push_back(new_request);
    }else{
        stat_.push_back(Request{id,nullptr});
    }
}

void JsonReader::MakeMapStatRequest(const json::Dict &dict){
    int id = 0;
    for(const auto &[key,value]: dict){
        if(key == "id"){
            id = value.AsInt();
        }
    }

    map_renderer_ptr_->SetRenderSettings(render_settings_);
    rh_.DrawBusRoute(std::move(map_renderer_ptr_));
    Request new_request{id,oss_.str()};
    stat_.push_back(new_request);
}

void JsonReader::MakeRouteStatRequest(const json::Dict &dict){
    std::string from = "";
    std::string to = "";
    int id = 0;
    for(const auto &[key,value]: dict){
        if(key == "id"){
            id = value.AsInt();
        }
        if(key == "from"){
            from = std::get<std::string>(value.GetValue());
        }
        if(key == "to"){
            to = std::get<std::string>(value.GetValue());
        }
    }
    main::RouteStat route_stat = {0,{}};
    const std::pair<std::string,std::string> stops = { from, to };
    if(rh_.GetRouteStat(route_stat,stops)){
        Request new_request{id,std::move(route_stat)};
        stat_.push_back(new_request);
    }else{
        Request new_request{id,nullptr};
        stat_.push_back(new_request);
    }

}

std::string JsonReader::ArrayToString(json::Array node) {
    std::string color = "none";
    std::vector<std::string> colors;
    for(const json::Node & node_str : node){
        if(node_str.IsInt()){
            colors.push_back(std::to_string(node_str.AsInt()));
        }else if(node_str.IsDouble()){
            std::stringstream ss;
            ss << std::setprecision(6);
            ss << node_str.AsDouble();
            colors.push_back(ss.str());
        }
    }
    if(colors.size() == 3){
        color="rgb(";
    }else if(colors.size() == 4){
        color="rgba(";
    }
    for(auto iter  = colors.begin(); iter != colors.end(); ++iter){
        color += *iter;
        if(iter != colors.end()-1){
            color +=",";
        }
    }
    color += ")";
    return color;
}
void JsonReader::SetRenderSettings(json::Node nodes){
    RenderSettings render_settings = RenderSettings();
    if(nodes.IsDict()){
        for(const auto &[key,value]: nodes.AsDict()){
            if(key == "width" ){
                render_settings.width = value.AsDouble();
            }
            if(key == "height" ){
                render_settings.height = value.AsDouble();
            }
            if(key == "padding" ){
                render_settings.padding = value.AsDouble();
            }
            if(key == "padding" ){
                render_settings.padding = value.AsDouble();
            }
            if(key == "stop_radius" ){
                render_settings.stop_radius = value.AsDouble();
            }
            if(key == "line_width" ){
                render_settings.line_width = value.AsDouble();
            }
            if(key == "bus_label_font_size" ){
                render_settings.bus_label_font_size = value.AsInt();
            }
            if(key == "bus_label_offset" ){
                if(value.IsArray()){
                    render_settings.bus_label_offset = {value.AsArray()[0].AsDouble(),value.AsArray()[1].AsDouble()};
                }
            }
            if(key == "stop_label_font_size" ){
                render_settings.stop_label_font_size = value.AsInt();
            }
            if(key == "stop_label_offset" ){
                if(value.IsArray()){
                    render_settings.stop_label_offset = {value.AsArray()[0].AsDouble(),value.AsArray()[1].AsDouble()};
                }
            }
            if(key == "underlayer_color" ){
                if(value.IsArray()){
                    render_settings.underlayer_color = ArrayToString(value.AsArray());
                }
                if(value.IsString()){
                    render_settings.underlayer_color = std::get<std::string>(value.GetValue());
                }
            }
            if(key == "underlayer_width" ){
                render_settings.underlayer_width = value.AsDouble();
            }

            if(key == "color_palette" ){
                if(value.IsArray()){
                    std::vector<std::string> colors;
                    for(const json::Node & node_str : value.AsArray()){
                        if(node_str.IsString()){
                            colors.push_back(std::get<std::string>(node_str.GetValue()));
                        }else{
                            colors.push_back(ArrayToString(node_str.AsArray()));
                        }
                    }
                    render_settings.color_palette = colors;
                }
            }
        }
    }

    render_settings_ = render_settings;
}

void JsonReader::SetRoutingSettings(json::Node node){
    int bus_velocity = 0;
    int bus_wait_time = 0;
    if(node.IsDict()){
        for(const auto &[key,value]: node.AsDict()){
            if(key == "bus_velocity"){
                bus_velocity = value.AsInt();
            }
            if(key == "bus_wait_time"){
                bus_wait_time = value.AsInt();
            }
        }
    }
}



struct VariantPrinter {
    json::Array* arr_ptr;
    int key;
    void operator()(std::nullptr_t)  {
        json::Node dict_node = json::Builder{}.StartDict()
                             .Key("request_id"s)
                             .Value(key)
                             .Key("error_message"s)
                             .Value("not found"s)
                             .EndDict()
                             .Build();

         arr_ptr->push_back(std::move(dict_node));
    }
    void operator()(main::BusStat bus_stat) {
        json::Node dict_node = json::Builder{}.StartDict()
            .Key("curvature"s)
            .Value(bus_stat.curvature)
            .Key("request_id"s)
            .Value(key)
            .Key("route_length"s)
            .Value(bus_stat.route_length)
            .Key("stop_count"s)
            .Value(bus_stat.stops_on_route)
            .Key("unique_stop_count"s)
            .Value(bus_stat.uniq_stops_on_route)
            .EndDict()
            .Build();
        arr_ptr->push_back(std::move(dict_node));
    }
    void operator()(std::set<std::string_view> stop_info)  {
        json::Array array_stop_info;
        array_stop_info.reserve(stop_info.size());
        for(std::string_view sv: stop_info){
            array_stop_info.push_back(json::Node(std::string(sv)));
        }
        json::Node dict_node{json::Dict{{"buses"s, json::Node(array_stop_info)}, {"request_id"s, key}}};
        arr_ptr->push_back(dict_node);
    }

    void operator()(std::string map_str)  {
        json::Node dict_node{json::Dict{{"map"s, map_str}, {"request_id"s, key}}};
        arr_ptr->push_back(dict_node); 
    }
    void operator()(main::RouteStat route_stat)  {
        json::Array items;
        for (main::RouteItem& item : route_stat.route_stat){
            json::Node dict_item;
            if(item.type == "Wait"){
                dict_item = json::Builder{}.StartDict()
                    .Key("stop_name"s)
                    .Value(dynamic_cast<main::RouteItemWait*>(&item)->stop_name )
                    .Key("time"s)
                    .Value(dynamic_cast<main::RouteItemWait*>(&item)->time)
                    .Key("type"s)
                    .Value(dynamic_cast<main::RouteItemWait*>(&item)->type)
                .EndDict().Build();
            }

            if(item.type == "Bus"){
                dict_item = json::Builder{}.StartDict()
                .Key("bus"s)
                    .Value(dynamic_cast<main::RouteItemBus*>(&item)->bus )
                    .Key("span_count"s)
                    .Value(dynamic_cast<main::RouteItemBus*>(&item)->span_count)
                    .Key("time"s)
                    .Value(dynamic_cast<main::RouteItemBus*>(&item)->time)
                    .Key("type"s)
                    .Value(dynamic_cast<main::RouteItemWait*>(&item)->type)
                    .EndDict().Build();
            }
            items.push_back(std::move(dict_item));
        }


        json::Node dict_node = json::Builder{}.StartDict()
            .Key("request_id"s)
            .Value(key)
            .Key("total_time"s)
            .Value(route_stat.total_time)
            .Key("items"s)
            .Value(items)
            .EndDict()
            .Build();

        arr_ptr->push_back(std::move(dict_node));

    }


};

void JsonReader::PrintStatRequests(){
    json::Array arr;
    if(stat_.size() >0){
        for(auto &request : stat_){
            std::visit(VariantPrinter{&arr, request.id}, request.info);

        }
    }
    json::Document document = json::Document(json::Node(arr));

    json::Print(document,out_);

}



domain::Stop JsonReader::ParseStop(const json::Dict& dict) const{
    std::string name="";
    double lat=0,lon=0;
    for(const auto &[key,value]: dict){
        if(key == "name"){
            if(value.IsString()){
                name =  std::get<std::string>(value.GetValue());// value.AsString();
            }
        }
        if(key == "latitude"){
            if(value.IsDouble()){
                lat = value.AsDouble();
            }
        }
        if(key == "longitude"){
            if(value.IsDouble()){
                lon = value.AsDouble();
            }
        }
    }
    return Stop{name,geo::Coordinates{lat,lon},{} };
}

Bus JsonReader::ParseBus(const json::Dict &dict) const{
    std::string name="";
    bool is_roundtrip=false;
    std::vector<std::string_view>  stops;
    for(const auto &[key,value]: dict){
        if(key == "name"){
            if(value.IsString()){
                name = std::get<std::string>(value.GetValue());
            }
        }
        if(key == "is_roundtrip"){
            if(value.IsBool()){
                is_roundtrip = value.AsBool();
            }
        }

        if(key == "stops"){
            if(value.IsArray()){
                for(const auto &stop_name: value.AsArray() ){
                    if(stop_name.IsString()){
                        const Stop* stop = db_.FindBusStopByName( std::get<std::string>(stop_name.GetValue()));
                        if(stop != nullptr){
                          stops.push_back(stop->name);
                        }

                    }
                }
            }
        }


    }


    return {name,stops,is_roundtrip};
}


} //end namespace transport_catalogue
