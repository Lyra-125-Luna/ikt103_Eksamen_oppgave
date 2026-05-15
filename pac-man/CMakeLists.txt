cmake_minimum_required(VERSION 4.0)
project(pac_man)

set(CMAKE_CXX_STANDARD 23)

add_executable(pac_man main.cpp
        game.cpp
        game.h
        map/map.cpp
        map/map.h
        object/object.h
        object/pac-man.h
        object/Ghosts.h
        object/layer.h
        data/audio/sounds.cpp
        data/audio/sounds.h)

target_include_directories(pac_man PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
target_link_libraries(pac_man PRIVATE SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System)
