#pragma once
#include <utility>
#include <string>
#include <vector>

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <unordered_map>
#include "domain.h"

namespace transport_catalogue {
using namespace svg;

class SphereProjector;

struct RenderSettings{
    double width = 1200.0;
    double height = 1200.0;
    double padding = 50.0;
    double line_width = 14.0;
    double stop_radius = 5.0;
    int bus_label_font_size = 20;
    std::pair<double,double> bus_label_offset = {7.0, 15.0};
    int stop_label_font_size = 20;
    std::pair<double,double> stop_label_offset = {7.0, -3.0};
    Color underlayer_color = "[255, 255, 255, 0.85]";
    double underlayer_width = 3.0;
    std::vector<Color> color_palette = {"green","[255, 160, 0]","red"};
};

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class MapRenderer{
    using StopsMap = std::map<std::string_view,domain::BusRoute>;
public:
    MapRenderer(std::ostream& out):out_(out){};
    void SetRenderSettings(RenderSettings& render_settings ){
        render_settings_ = render_settings;
    }
    void RenderMap(StopsMap bus_stops);
private:
    void RenderPolylines(const StopsMap &bus_stops,
                         const std::vector<std::string>& color_palette,
                         const SphereProjector& proj,
                         svg::Document& doc);

    void RenderRouteNames(const StopsMap &bus_stops,
                         const std::vector<std::string>& color_palette,
                         const SphereProjector& proj,
                         svg::Document& doc);

    std::map<std::string_view,geo::Coordinates> RenderRouteCircles(const StopsMap &bus_stops,
                          const SphereProjector& proj,
                          svg::Document& doc);

    void RenderStopNames(const std::map<std::string_view,geo::Coordinates>& stops_map,
                          const SphereProjector& proj,
                          svg::Document& doc);

    std::string StringToRgb(std::string color_str);
    void VectorStringToRgb(std::vector<std::string>& color_vec);
    RenderSettings render_settings_;
    std::ostream& out_;
};

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


}// end namespace
