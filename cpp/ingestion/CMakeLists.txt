add_library(ingestion STATIC
    udp_server.cpp
)

target_include_directories(ingestion PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ingestion
    aggregation
)
