cmake_minimum_required(VERSION 3.25)

set(VCPKG_OVERLAY_PORTS ${CMAKE_CURRENT_LIST_DIR}/overlays)
message("VCPKG_OVERLAY_PORTS ${VCPKG_OVERLAY_PORTS}")

project(polarspectrogram)
add_subdirectory(stripack/stripack_c)
add_subdirectory(stripack/stripack_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME}
        main.cpp
        Plot.cpp
        PlotWindow.cpp
)

find_package(Qt5
        COMPONENTS
        Widgets
        REQUIRED
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt::Widgets
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("use debug lib")
    set(subfolder debug)
else()
    set(subfolder "")
endif()

find_package(Qwt REQUIRED)

message("Qwt_DIR ${Qwt_DIR}")
message("Qwt_INCLUDE_DIR ${Qwt_INCLUDE_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE stripack/stripack_cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Qwt::qwt stripack_cpp)

