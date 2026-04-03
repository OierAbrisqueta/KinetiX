cmake_minimum_required(VERSION 3.20)
project(KinetiX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(KinetiX_Server
        src/server/main.c
        src/server/gestor_bd.c
        src/server/gestor_config.c
        src/server/gestor_log.c
        src/server/menu.c
        src/server/sqlite3.c
        src/server/gestor_config.c
        include/informes.h
)

target_include_directories(KinetiX_Server PRIVATE
        include
)

if (UNIX)
    target_link_libraries(KinetiX_Server PRIVATE dl pthread m)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/reportes)

configure_file(
        ${CMAKE_SOURCE_DIR}/data/config.conf
        ${CMAKE_BINARY_DIR}/data/config.conf
        COPYONLY
)

configure_file(
        ${CMAKE_SOURCE_DIR}/data/KinetiX.sqlite
        ${CMAKE_BINARY_DIR}/data/KinetiX.sqlite
        COPYONLY
)
