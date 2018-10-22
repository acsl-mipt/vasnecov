cmake_minimum_required(VERSION 2.8.11)

project(dronesformation)

find_package(Qt5Core    REQUIRED)
find_package(Qt5OpenGL  REQUIRED)
find_package(Qt5Gui     REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(
    SYSTEM
    ../../src
)
link_directories(
    ../../build/
    ../../build/thirdparty/bmcl/src/bmcl
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(DRONESFORMATION_SOURCES
    dronesformation.cpp
    BomberModel.cpp
    ProductModel.cpp
    Scene.cpp
    )

add_executable(dronesformation ${DRONESFORMATION_SOURCES})

if(NOT MSVC)
    target_compile_options(dronesformation PUBLIC -std=c++11)
    target_compile_options(dronesformation PRIVATE -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual)
endif()

qt5_use_modules(dronesformation Core OpenGL Gui Widgets)
target_link_libraries(dronesformation Vasnecov bmcl)

if(NOT MSVC)
    target_link_libraries(dronesformation GL GLU)
else()
    target_link_libraries(dronesformation OpenGL32 GLU32)
endif()
