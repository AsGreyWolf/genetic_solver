cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(gpugen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Threads)
# find_package(OpenCL REQUIRED)
# add_definitions(${OpenCL_DEFINITIONS})
# include_directories(${OpenCL_INCLUDES})
find_package(Boost COMPONENTS system REQUIRED)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
add_definitions(${Qt5Widgets_DEFINITIONS})
include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Charts_DEFINITIONS})
include_directories(${Qt5Charts_INCLUDES})
file(GLOB_RECURSE main_SRC
    "src/*.hpp"
    "src/*.cpp"
)
add_executable(main ${main_SRC})
target_link_libraries(main Boost::system Threads::Threads Qt5::Widgets Qt5::Charts gmp) #OpenCL::OpenCL
