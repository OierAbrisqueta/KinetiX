# KinetiX

KinetiX is a system that allows to manage the renting of vehicles: bicycles and scooters. It features a local administration console and a remote client-server architecture.

## Components

The project is built using C and C++, via CMake, and consists of three main executables:

*   **KinetiX_Admin**: A local command line administration tool for managing the system and database directly (C).
*   **KinetiX_Server**: A multithreaded TCP backend server that handles remote client connections (C).
*   **KinetiX_Client**: A remote command line client allowing end users to rent and return vehicles (C++).

## Requirements

*   CMake 3.20 or higher
*   C11 and C++17 compatible compiler
*   POSIX Threads and Sockets (Linux/macOS) or Winsock2 (Windows)

*(Note: SQLite3 is included directly in the source code)*

## Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running the Application

After building, the executables and necessary `data/` assets will be placed in your build directory. 

1. Start the remote server or use the local admin tool:
   ```bash
   ./KinetiX_Server
   # or
   ./KinetiX_Admin
   ```
   *(For the KinetiX_Admin: Usuario: admin and Contrasena: 1234)*


2. Open another terminal and start the client to connect to the server:
   ```bash
   ./KinetiX_Client
   ```
   *(DNI: 12345678Z and Contrasena: 1234)*
