#pragma once
#include "json.h"
#include <iostream>
#include <string>
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include <sstream>
#include <cassert>
#include "map_renderer.h"

namespace transport_catalogue {
class JsonReader{
public:
    JsonReader(const json::Document& document,main::TransportCatalogue& db,
               const RequestHandler& rh ,std::ostream& out):
        document_(document),db_(db),rh_(rh),out_(out){

        map_renderer_ptr_ = std::make_unique<MapRenderer>(oss_);
        //MapRenderer map_renderer(oss);

        auto root_node = document_.GetRoot();

        if(root_node.IsDict()){

           for (const auto&[ str, node] : root_node.AsDict()){
                //std::cout << str << std::endl;

               if(str == "base_requests"){
                    MakeBaseRequests(node);
               }
               if(str == "stat_requests"){
                   MakeStatRequests(node);
               }
               if(str == "render_settings"){
                   SetRenderSettings(node);
               }
               if(str == "routing_settings"){
                   SetRoutingSettings(node);
               }

           }
        }else{
        assert(false && "root node not map");
        }
    }

    using stat_info = std::variant<std::nullptr_t,
                                   main::BusStat,
                                   std::set<std::string_view>,
                                   std::string,
                                   main::RouteStat>;

private:
    struct Request{
        int id;
        stat_info info;
    };

    std::string ArrayToString(json::Array node);
    void MakeBaseRequests(json::Node node) const;
    domain::Stop ParseStop(const json::Dict& dict) const;
    domain::Bus ParseBus(const json::Dict& dict) const;
    void MakeStatRequests(json::Node node);
    void MakeStopStatRequest(const json::Dict& dict);
    void MakeBusStatRequest(const json::Dict& dict);
    void MakeMapStatRequest(const json::Dict& dict);
    void MakeRouteStatRequest(const json::Dict& dict);
    void SetRenderSettings(json::Node node);
    void SetRoutingSettings(json::Node node);
    void PrintStatRequests();
    json::Document document_;
    main::TransportCatalogue& db_;
    const RequestHandler& rh_;
    std::unique_ptr<MapRenderer> map_renderer_ptr_;
    RenderSettings render_settings_;

    std::vector<Request> stat_;

    std::ostream& out_;
    std::ostringstream oss_;

};

} //end namespace
