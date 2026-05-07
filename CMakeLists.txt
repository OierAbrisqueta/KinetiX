cmake_minimum_required(VERSION 3.20)
project(KinetiX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES_COMUNES
        src/server/gestor_bd.c
        src/server/gestor_config.c
        src/server/gestor_log.c
        src/server/sqlite3.c
        src/server/hash_utils.c
)

# Ejecutable 1: administrador local (Fase 1)
add_executable(KinetiX_Admin
        src/server/main.c
        src/server/menu.c
        ${SOURCES_COMUNES}
        include/informes.h
)
target_include_directories(KinetiX_Admin PRIVATE include)
target_link_libraries(KinetiX_Admin PRIVATE ws2_32)

# Ejecutable 2: servidor remoto (Fase 2)
add_executable(KinetiX_Server
        src/server/main_server.c
        src/server/server_socket.c
        src/server/protocolo_servidor.c
        ${SOURCES_COMUNES}
        include/informes.h
)
target_include_directories(KinetiX_Server PRIVATE include)
target_link_libraries(KinetiX_Server PRIVATE ws2_32)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/reportes)

configure_file(${CMAKE_SOURCE_DIR}/data/config.conf
        ${CMAKE_BINARY_DIR}/data/config.conf COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/data/sentencias_creacion_bd.sql
        ${CMAKE_BINARY_DIR}/data/sentencias_creacion_bd.sql COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/data/datos_de_prueba.sql
        ${CMAKE_BINARY_DIR}/data/datos_de_prueba.sql COPYONLY)