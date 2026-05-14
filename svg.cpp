#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Polyline &Polyline::AddPoint(Point point)
{
    vector_points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t i=0;
    for(Point p:vector_points_ ){
        out << p.x << "," << p.y;
        if(i < vector_points_.size()-1){
            out << " ";
        }
        ++i;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;

}

void Document::Render(std::ostream &out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for(auto iter= objects_.begin(); iter != objects_.end(); ++iter){
        if((*iter) != nullptr){
            (*iter)->Render(out);
        }
    }
    out << "</svg>"sv;

}

void Text::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x=\"" << pos_.x <<"\" y=\"" << pos_.y <<"\" ";
    out << "dx=\"" << offset_.x <<"\" dy=\"" << offset_.y <<"\"";
    //font-size="12" font-family="Verdana" font-weight="bold"
    if(size_ > 0 ){
        out << " font-size=\"" << std::to_string(size_) << "\"";
    }

    if(font_family_.length() > 0 ){
        out << " font-family=\"" << font_family_ << "\"";
    }
    if(font_weight_.length() > 0  ){
        out << " font-weight=\"" << font_weight_ << "\"";
    }

    out << ">"sv;
    out << format_data(data_);
    out << "</text>"sv;
}

std::string Text::format_data(std::string_view data) const{
    std::string result = "";
    std::string needle ="\"'<>&";
    size_t pos = 0;
    size_t find_pos = data.find_first_of(needle,pos);
    if(find_pos == std::string::npos){
        return std::string(data);
    }
    while(find_pos != std::string::npos){
        result += std::string(data.substr(pos,find_pos - pos));
        char ch = data[find_pos];
        if(ch == '>'){
            result +="&gt;";
        }

        if(ch == '<'){
            result +="&lt;";
        }

        if(ch == '\''){
            result +="&apos;";
        }

        if(ch == '"'){
            result +="&quot;";
        }

        if(ch == '&'){
            result +="&amp;";
        }

        pos = find_pos+1;
        find_pos = data.find_first_of(needle,pos);
    }
    result += std::string(data.substr(pos,data.size() - pos));
    return result;
}

}  // namespace svg

