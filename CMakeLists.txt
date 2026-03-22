cmake_minimum_required(VERSION 3.20)
project(KinetiX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(kinetix
        src/server/gestor_bd.c
        src/server/sqlite3.c
)

target_include_directories(kinetix PRIVATE
        include
)
