cmake_minimum_required(VERSION 3.7)
project(LD50 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if(WIN32)
    set(SDL2_INCLUDE_DIR "C:/Program Files/mingw64/include/SDL2")
    set(SDL2_LIBRARY_DIR "C:/Program Files/mingw64/lib")
    set(SDL2_IMAGE_INCLUDE_DIR "C:/SDL_image/include/SDL2")
    set(SDL2_IMAGE_LIBRARY_DIR "C:/SDL_image/lib")
    set(SDL2_TTF_INCLUDE_DIR "C:/SDL_ttf/include/SDL2")
    set(SDL2_TTF_LIBRARY_DIR "C:/SDL_ttf/lib")
elseif(APPLE)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2TTF REQUIRED)
endif()

set(SOURCE_INCLUDES
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Engine
    ${PROJECT_SOURCE_DIR}/src/Engine/Audio
    ${PROJECT_SOURCE_DIR}/src/Engine/Render
    ${PROJECT_SOURCE_DIR}/src/Engine/Localization
    ${PROJECT_SOURCE_DIR}/src/Engine/Input
    ${PROJECT_SOURCE_DIR}/src/Engine/RandomUtilities
    ${PROJECT_SOURCE_DIR}/src/States
    ${PROJECT_SOURCE_DIR}/src/Entity
    ${PROJECT_SOURCE_DIR}/src/Entity/Player
    ${PROJECT_SOURCE_DIR}/src/Entity/Enemies
    ${PROJECT_SOURCE_DIR}/src/Entity/Utilities
    ${PROJECT_SOURCE_DIR}/src/Level
    ${PROJECT_SOURCE_DIR}/src/Item
    )
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/Engine/Game.cpp
    ${PROJECT_SOURCE_DIR}/src/Engine/CollisionDetector.cpp
    ${PROJECT_SOURCE_DIR}/src/Engine/Audio/Audio.cpp
    ${PROJECT_SOURCE_DIR}/src/Engine/Render/Spritesheet.cpp
    ${PROJECT_SOURCE_DIR}/src/Engine/Render/Text.cpp
    ${PROJECT_SOURCE_DIR}/src/States/State.cpp
    ${PROJECT_SOURCE_DIR}/src/States/GameState.cpp
    ${PROJECT_SOURCE_DIR}/src/Engine/Input/Keyboard.cpp
    ${PROJECT_SOURCE_DIR}/src/Entity/Entity.cpp
    ${PROJECT_SOURCE_DIR}/src/Entity/Player/Player.cpp
    ${PROJECT_SOURCE_DIR}/src/Entity/Enemies/Grunt.cpp
    ${PROJECT_SOURCE_DIR}/src/Entity/Enemies/Bishop.cpp
    ${PROJECT_SOURCE_DIR}/src/Entity/Enemies/Tank.cpp
    ${PROJECT_SOURCE_DIR}/src/Level/Level.cpp
    ${PROJECT_SOURCE_DIR}/src/Level/Tile.cpp
    ${PROJECT_SOURCE_DIR}/src/Item/Bomb.cpp
    )

if(WIN32)
    include_directories(${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR} ${SOURCE_INCLUDES})
    add_executable(LD50 ${SOURCES})
    # remove -mconsole for release builds
    target_link_libraries(LD50 -lmingw32 ${SDL2_LIBRARY_DIR}/libSDL2main.a ${SDL2_LIBRARY_DIR}/libSDL2.dll.a ${SDL2_IMAGE_LIBRARY_DIR}/libSDL2_image.dll.a ${SDL2_TTF_LIBRARY_DIR}/libSDL2_ttf.dll.a -mwindows -mconsole)
elseif(APPLE)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIR} ${SOURCE_INCLUDES})
    add_executable(LD50 ${SOURCES})
    target_link_libraries(LD50 ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2TTF_LIBRARY})
endif()

add_custom_command(TARGET LD50 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${PROJECT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:LD50>/res/)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
