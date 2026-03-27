cmake_minimum_required(VERSION 3.22)
project(nucleus-shell VERSION 1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src/utils
    ${PROJECT_SOURCE_DIR}/src/cmd
)

# Source files
set(SOURCES
    src/main.cpp
    src/cmd/install/install.cpp
    src/cmd/ipc/ipc.cpp
    src/cmd/theme/theme.cpp
    src/cmd/update/update.cpp
)

# Executable
add_executable(nucleus ${SOURCES})

# Link libraries
find_package(CURL REQUIRED)
target_link_libraries(nucleus PRIVATE CURL::libcurl pthread)

install(TARGETS nucleus
        RUNTIME DESTINATION bin)
