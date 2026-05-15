#include "domain.h"
#include <map>
namespace transport_catalogue {
class TransportRouter{
public:
    TransportRouter(){ }
    void AddStop();
    ~TransportRouter() {

    }
private:
    std::map<std::string,domain::StopEdge> vertex_stop_;
    size_t last_stop_index_ = 0;
};


} //end tc namespace
