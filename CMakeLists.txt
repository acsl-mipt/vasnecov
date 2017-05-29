cmake_minimum_required(VERSION 2.8.11)

project(Vasnecov)

find_package(Qt5Core    REQUIRED)
find_package(Qt5OpenGL  REQUIRED)
find_package(Qt5Gui     REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_subdirectory(thirdparty/bmcl)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(VASNECOV_SOURCES
    src/libVasnecov/configuration.h
    src/libVasnecov/coreobject.h
    src/libVasnecov/elementlist.h
    src/libVasnecov/technologist.h
    src/libVasnecov/technologist.cpp
    src/libVasnecov/types.h
    src/libVasnecov/vasnecov.h
    src/libVasnecov/vasnecov.cpp
    src/libVasnecov/vasnecovelement.h
    src/libVasnecov/vasnecovelement.cpp
    src/libVasnecov/vasnecovfigure.h
    src/libVasnecov/vasnecovfigure.cpp
    src/libVasnecov/vasnecovlabel.h
    src/libVasnecov/vasnecovlabel.cpp
    src/libVasnecov/vasnecovlamp.h
    src/libVasnecov/vasnecovlamp.cpp
    src/libVasnecov/vasnecovmaterial.h
    src/libVasnecov/vasnecovmaterial.cpp
    src/libVasnecov/vasnecovmesh.h
    src/libVasnecov/vasnecovmesh.cpp
    src/libVasnecov/vasnecovpipeline.h
    src/libVasnecov/vasnecovpipeline.cpp
    src/libVasnecov/vasnecovproduct.h
    src/libVasnecov/vasnecovproduct.cpp
    src/libVasnecov/vasnecovscene.h
    src/libVasnecov/vasnecovscene.cpp
    src/libVasnecov/vasnecovtexture.h
    src/libVasnecov/vasnecovtexture.cpp
    src/libVasnecov/vasnecovuniverse.h
    src/libVasnecov/vasnecovuniverse.cpp
    src/libVasnecov/vasnecovworld.h
    src/libVasnecov/vasnecovworld.cpp
    src/libVasnecov/version.h
    src/Vasnecov
    src/VasnecovUniverse
    src/VasnecovScene
    src/VasnecovProduct
    )

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(VASNECOV_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/src
        PARENT_SCOPE
    )
endif()

add_library(Vasnecov STATIC ${VASNECOV_SOURCES} resources.qrc)

if(NOT MSVC)
    target_compile_options(Vasnecov PUBLIC -std=c++11)
    target_compile_options(Vasnecov PRIVATE -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual -Weffc++)
endif()

qt5_use_modules(Vasnecov Core OpenGL Gui Widgets)

target_link_libraries(Vasnecov bmcl)

if(NOT MSVC)
    target_link_libraries(Vasnecov GL GLU)
else()
    target_link_libraries(Vasnecov OpenGL32 GLU32)
endif()
