add_library(aggregation STATIC
    shard.cpp
)

target_include_directories(aggregation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
