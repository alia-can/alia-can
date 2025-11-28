cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(alia-can
    VERSION 0.0.1
    DESCRIPTION "AliaCan - Shell Alias Manager with Auto-Backup"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global compile warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Qt auto-settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# ----------------------------------------------------------------------
# APP SOURCES
# ----------------------------------------------------------------------

set(APP_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/shelldetector.cpp
    src/aliasmanager.cpp
    src/configfilehandler.cpp
    src/backupmanager.cpp
)

set(APP_HEADERS
    src/mainwindow.hpp
    src/shelldetector.hpp
    src/aliasmanager.hpp
    src/configfilehandler.hpp
    src/backupmanager.hpp
)

add_executable(alia-can
    ${APP_SOURCES}
    ${APP_HEADERS}
)

target_link_libraries(alia-can
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_include_directories(alia-can PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ----------------------------------------------------------------------
# TEST TARGET
# ----------------------------------------------------------------------

enable_testing()

set(TEST_SOURCES
    tests/main.cpp             # Single test entry point
    tests/test_shelldetector.cpp
    tests/test_aliasmanager.cpp
    tests/test_confighandler.cpp

    # Core source logic reused by tests
    src/shelldetector.cpp
    src/aliasmanager.cpp
    src/configfilehandler.cpp
    src/backupmanager.cpp
)

add_executable(alia-can-tests ${TEST_SOURCES})

target_include_directories(alia-can-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# No Qt linked for tests unless you need GUI testing
add_test(NAME AliaCan-Tests COMMAND alia-can-tests)

# ----------------------------------------------------------------------
# INSTALLATION
# ----------------------------------------------------------------------

install(TARGETS alia-can DESTINATION /usr/local/bin)

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# ----------------------------------------------------------------------
# BUILD INFO OUTPUT
# ----------------------------------------------------------------------

message(STATUS "========================================")
message(STATUS "AliaCan Configuration")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt6 Found: ${Qt6_FOUND}")
message(STATUS "========================================")
