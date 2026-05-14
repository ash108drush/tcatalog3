#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265558979223846
#endif
#pragma once
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
namespace svg {

class Document;
class Object;

using Color = std::string;
inline const Color NoneColor{"none"};
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    using namespace std::literals;
    switch (line_cap) {
    case StrokeLineCap::BUTT:
        out << "butt"s;
        break;
    case StrokeLineCap::ROUND:
        out << "round"s;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"s;
        break;
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    using namespace std::literals;
    switch (line_join) {
    case StrokeLineJoin::ARCS:
        out << "arcs"s;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"s;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"s;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"s;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"s;
        break;

    }
    return out;
}
template <typename Owner>
class PathProps{
public:

    Owner& SetFillColor(Color color){
        fill_color_ = color;
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color){
        stroke_color_ = color;
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width){
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }

        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }
    std::optional<Color> fill_color_ = std::nullopt;
    std::optional<Color> stroke_color_ = std::nullopt;
    std::optional<double> stroke_width_ = std::nullopt;
    std::optional<StrokeLineCap> line_cap_ = std::nullopt;
    std::optional<StrokeLineJoin> line_join_ = std::nullopt;

};

class ObjectContainer{
public:
    template <typename ObjectType>
    void Add(ObjectType object) {
        AddPtr(std::make_unique<ObjectType>(std::move(object)));
    }
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    virtual ~ObjectContainer() = default;

};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};


/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline>{
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);
    void RenderObject(const RenderContext& context) const override;

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    std::vector<Point> vector_points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text: public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos){
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset){
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size){
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family){
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight){
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data){
        data_ = data;
        return *this;
    }
    void RenderObject(const RenderContext& context) const override;

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    std::string format_data(std::string_view ) const ;
    std::string data_="";
    std::string font_weight_="";
    std::string font_family_="";
    uint32_t size_ = 1;
    Point offset_={0.0, 0.0};
    Point pos_={0.0, 0.0};

};

class Document :public ObjectContainer{
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override{
        objects_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg



namespace shapes {

class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable, public svg::PathProps<Star>  {
public:
    Star(svg::Point  center, double outer_rad, double inner_rad, int num_rays):center_(center),
        outer_rad_(outer_rad),inner_rad_(inner_rad),num_rays_(num_rays) {
    }
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(CreateStar());
    }

private:
    svg::Polyline CreateStar() const{
        svg::Polyline polyline;
        for (int i = 0; i <= num_rays_; ++i) {
            double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
            polyline.AddPoint({center_.x + outer_rad_ * sin(angle), center_.y - outer_rad_ * cos(angle)});
            if (i == num_rays_) {
                break;
            }
            angle += M_PI / num_rays_;
            polyline.AddPoint({center_.x + inner_rad_ * sin(angle), center_.y - inner_rad_ * cos(angle)});
        }
        return polyline.SetFillColor("red").SetStrokeColor("black");
    }

    svg::Point center_;
    double outer_rad_;
    double inner_rad_;
    int num_rays_;

};
class Snowman : public svg::Drawable, public svg::PathProps<Snowman> {
public:
    Snowman(svg::Point  center, double head_radius):center_(center),head_radius_(head_radius){
        svg::Circle circle;
        snowman_.push_back(circle.SetCenter({center_.x,center_.y+head_radius_ * 5} ).SetRadius(head_radius_ * 2).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        snowman_.push_back(circle.SetCenter({center_.x,center_.y+head_radius_ * 2} ).SetRadius(head_radius_ * 1.5).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        snowman_.push_back(circle.SetCenter(center_).SetRadius(head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
    }

    void Draw(svg::ObjectContainer& container) const override {
        for(const auto& ball:snowman_){
            container.Add(ball);
        }
    }

private:
    std::vector<svg::Circle> snowman_;
    svg::Point  center_;
    double head_radius_;
};

} // namespace shapes
