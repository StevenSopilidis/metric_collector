cmake_minimum_required(VERSION 3.20)

project(
    metric_collector
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------
# Default build type (if not provided)
# ----------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type: Debug Release RelWithDebInfo MinSizeRel"
        FORCE
    )
endif()

# check for lto support
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if (ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO not supported: ${ipo_error}")
endif()

# sanitizer
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
    # -O3
    # -march=native
    # -mtune=native
    -Wall
    -Wextra
    -Wpedantic
)

add_subdirectory(ingestion)
add_subdirectory(aggregation)


add_executable(collector
    main.cpp
)

target_link_libraries(collector
    ingestion
    aggregation
)

target_link_libraries(collector
    pthread
)