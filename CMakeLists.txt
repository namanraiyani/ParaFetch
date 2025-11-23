cmake_minimum_required(VERSION 3.16)
project(ParaFetch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

add_executable(ParaFetch
    main.cpp
    myform.cpp
    myform.h
    downloadworker.cpp
    downloadworker.h
    downloadmanager.cpp
    downloadmanager.h
    httphelper.cpp
    httphelper.h
    chunkprogress.h
)

target_link_libraries(ParaFetch PRIVATE Qt6::Core Qt6::Widgets ${CURL_LIBRARIES})
target_include_directories(ParaFetch PRIVATE ${CURL_INCLUDE_DIRS})