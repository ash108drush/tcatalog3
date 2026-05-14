#include "map_renderer.h"
#include <iostream>
#include <vector>

namespace transport_catalogue {

void MapRenderer::RenderMap(StopsMap  bus_stops){
    std::vector<geo::Coordinates> geo_coords;
    geo_coords.reserve(bus_stops.size());
    for(auto const &[bus_name, bus_data] :bus_stops){
        for(auto const &stop: bus_data.stops){
            geo_coords.push_back(stop->coordinates);
        }
    }
    const SphereProjector proj{
                               geo_coords.begin(), geo_coords.end(),
                               render_settings_.width,
                               render_settings_.height,
                               render_settings_.padding };

    svg::Document doc;
    std::vector<std::string> color_palette = render_settings_.color_palette;
    VectorStringToRgb(color_palette);
    if(bus_stops.size() == 0){
        doc.Render(out_);
        return;
    }
    RenderPolylines(bus_stops, color_palette, proj, doc);
    RenderRouteNames(bus_stops, color_palette, proj, doc);
    std::map<std::string_view,geo::Coordinates> stops_map = RenderRouteCircles(bus_stops,proj,doc);
    RenderStopNames(stops_map, proj, doc);
    doc.Render(out_);
}

void MapRenderer::RenderRouteNames(const StopsMap & bus_stops,
                                  const std::vector<std::string>& color_palette,
                                  const SphereProjector& proj,
                                  svg::Document& doc){
    int color_index = 0;

    for(auto const &[bus_name, bus_data] :bus_stops){
        svg::Text text = svg::Text();
        svg::Text text_underlayer = svg::Text();
        text_underlayer.SetFillColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetFillColor(color_palette[color_index % color_palette.size()]);

        text_underlayer.SetOffset({render_settings_.bus_label_offset.first,render_settings_.bus_label_offset.second});
        text.SetOffset({render_settings_.bus_label_offset.first,render_settings_.bus_label_offset.second});
        text.SetFontSize(render_settings_.bus_label_font_size);
        text_underlayer.SetFontSize(render_settings_.bus_label_font_size);
        text.SetFontFamily("Verdana");
        text_underlayer.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text_underlayer.SetFontWeight("bold");
        text.SetData(static_cast<std::string>(bus_name));
        text_underlayer.SetData(static_cast<std::string>(bus_name));

        if(bus_data.is_roundtrip){
            Point p = proj(bus_data.stops[0]->coordinates);
            text_underlayer.SetPosition(p);
            text.SetPosition(p);
            doc.Add(text_underlayer);
            doc.Add(text);
            ++color_index;
        }else{
            Point p1 = proj(bus_data.stops[0]->coordinates);
            text_underlayer.SetPosition(p1);
            text.SetPosition(p1);
            doc.Add(text_underlayer);
            doc.Add(text);
            if(bus_data.last_stop != bus_data.stops[0] ){
                Point p2 = proj(bus_data.last_stop->coordinates);
                text_underlayer.SetPosition(p2);
                text.SetPosition(p2);
                doc.Add(text_underlayer);
                doc.Add(text);
            }

            ++color_index;
        }
    }
}

std::map<std::string_view,geo::Coordinates> MapRenderer::RenderRouteCircles(
    const StopsMap &bus_stops,
    const SphereProjector& proj,
    svg::Document& doc){

    svg::Circle circle = svg::Circle();
    circle.SetRadius(render_settings_.stop_radius);
    circle.SetFillColor("white");
    std::map<std::string_view,geo::Coordinates> stops_map;
    for(auto const &[bus_name, bus_data] :bus_stops){
        for(const domain::Stop * stop : bus_data.stops){
            stops_map.insert({stop->name,stop->coordinates});
        }
    }
    for(const auto &[name,coords] : stops_map){
        circle.SetCenter(proj(coords));
        doc.Add(circle);
    }

    return stops_map;
}

void MapRenderer::RenderStopNames(const std::map<std::string_view,geo::Coordinates> &stops_map,
                                    const SphereProjector &proj, Document &doc){
    for(auto const &[stop_name, coords] :stops_map){
        svg::Text text = svg::Text();
        svg::Text text_underlayer = svg::Text();
        text_underlayer.SetOffset({render_settings_.stop_label_offset.first,render_settings_.stop_label_offset.second});
        text.SetOffset({render_settings_.stop_label_offset.first,render_settings_.stop_label_offset.second});
        text.SetFontSize(render_settings_.stop_label_font_size);
        text_underlayer.SetFontSize(render_settings_.stop_label_font_size);
        text.SetFontFamily("Verdana");
        text_underlayer.SetFontFamily("Verdana");

        text.SetData(static_cast<std::string>(stop_name));
        text_underlayer.SetData(static_cast<std::string>(stop_name));

        text_underlayer.SetFillColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetFillColor("black");

        Point p = proj(coords);
        text_underlayer.SetPosition(p);
        text.SetPosition(p);
        doc.Add(text_underlayer);
        doc.Add(text);

    }

}

void MapRenderer::RenderPolylines(const StopsMap & bus_stops,
                                  const std::vector<std::string>& color_palette,
                                  const SphereProjector& proj,
                                  svg::Document& doc){
    int color_index = 0;
    for(auto const &[bus_name, bus_data] :bus_stops){
        svg::Polyline poly_line = svg::Polyline();
        poly_line.SetFillColor("none");
        poly_line.SetStrokeWidth(render_settings_.line_width);
        poly_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        poly_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        poly_line.SetStrokeColor(color_palette[color_index % color_palette.size()]);

        for(auto const &stop: bus_data.stops){
            poly_line.AddPoint(proj(stop->coordinates));
        }

        doc.Add(poly_line);
        ++color_index;
    }
}

void MapRenderer::VectorStringToRgb(std::vector<Color> &color_palette){
    for(auto iter = color_palette.begin(); iter != color_palette.end();++iter){
        if(iter->find("[") != iter->npos){
            *iter = StringToRgb(*iter);
        }
    }
}

std::string MapRenderer::StringToRgb(std::string color_str){
    if(color_str.find("[") == color_str.npos) return color_str;
    int commas = std::count_if(color_str.begin(),color_str.end(),[](char c) { return c == ','; });
    if(commas == 2){
        size_t pos = color_str.find("[");
        color_str.replace(pos,1,"rgb(");
        pos = color_str.find("]");
        color_str.replace(pos,1,")");
        return color_str;
    }
    if(commas == 3){
        size_t pos = color_str.find("[");
        color_str.replace(pos,1,"rgba(");
        pos = color_str.find("]");
        color_str.replace(pos,1,")");
        return color_str;
    }

    return color_str;
}

}//end namespace
