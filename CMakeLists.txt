cmake_minimum_required(VERSION 2.8.11)

project(Vasnecov)

find_package(Qt5Core    REQUIRED)
find_package(Qt5OpenGL  REQUIRED)
find_package(Qt5Gui     REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(VASNECOV_SOURCES
    src/configuration.h
    src/coreobject.h
    src/elementlist.h
    src/types.h
    src/technologist.cpp
    src/vasnecov.cpp
    src/vasnecovelement.cpp
    src/vasnecovfigure.cpp
    src/vasnecovlabel.cpp
    src/vasnecovlamp.cpp
    src/vasnecovmaterial.cpp
    src/vasnecovmatrix4x4.cpp
    src/vasnecovmesh.cpp
    src/vasnecovpipeline.cpp
    src/vasnecovproduct.cpp
    src/vasnecovscene.cpp
    src/vasnecovtexture.cpp
    src/vasnecovuniverse.cpp
    src/vasnecovworld.cpp
    src/version.h
    )

add_library(Vasnecov STATIC ${VASNECOV_SOURCES} resources.qrc)

if(NOT MSVC)
    target_compile_options(Vasnecov PUBLIC -std=c++11)
    target_compile_options(Vasnecov PRIVATE -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual -Weffc++)
endif()

qt5_use_modules(Vasnecov Core OpenGL Gui Widgets)

if(NOT MSVC)
    target_link_libraries(Vasnecov GL GLU)
else()
    target_link_libraries(Vasnecov OpenGL32 GLU32)
endif()




