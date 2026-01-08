add_library(aggregation STATIC
    bucket.cpp
)

target_include_directories(aggregation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
