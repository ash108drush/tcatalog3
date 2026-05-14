#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
using namespace transport_catalogue;

class RequestHandler {
public:
    RequestHandler(const main::TransportCatalogue& db):db_(db){};


    void DrawBusRoute(std::unique_ptr<MapRenderer>&& map_renderer_ptr) const;
private:
    const main::TransportCatalogue& db_;
};

