cmake_minimum_required(VERSION 3.16)

project(EnvSensorServer
    VERSION 1.0.0
    DESCRIPTION "Environmental sensor daemon and client"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Put executables in <build>/bin, matching the old Makefile layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED)

# Code shared by the server and the client
add_library(envsensor_common STATIC
    src/my_socket_lib.cpp
    src/bme280.cpp
    src/SSNP.cpp
    src/CommandLineParser.cpp
)
target_include_directories(envsensor_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_compile_options(envsensor_common PUBLIC -Wall -Wextra)
target_link_libraries(envsensor_common PUBLIC Threads::Threads)

# Server daemon
add_executable(envsensord src/server.cpp)
target_link_libraries(envsensord PRIVATE envsensor_common)

# Command line client
add_executable(envsensor-cli src/client.cpp)
target_link_libraries(envsensor-cli PRIVATE envsensor_common)

include(GNUInstallDirs)
install(TARGETS envsensord envsensor-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
