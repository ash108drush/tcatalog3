cmake_minimum_required(VERSION 3.16)

project(tcatalog3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcatalog3 main.cpp
    domain.cpp domain.h geo.cpp geo.h graph.h json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h map_renderer.cpp map_renderer.h ranges.h request_handler.cpp request_handler.h router.h svg.cpp svg.h transport_catalogue.cpp transport_catalogue.h transport_router.cpp transport_router.h)

include(GNUInstallDirs)
install(TARGETS tcatalog3
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
