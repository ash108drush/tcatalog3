#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
using namespace transport_catalogue;

class RequestHandler {
public:
    RequestHandler(const main::TransportCatalogue& db):db_(db){};
    void DrawBusRoute(std::unique_ptr<MapRenderer>&& map_renderer_ptr) const;
    bool GetRouteStat(main::RouteStat& route_stat,const std::pair<std::string,std::string> stops) const;
private:
    const main::TransportCatalogue& db_;
};

