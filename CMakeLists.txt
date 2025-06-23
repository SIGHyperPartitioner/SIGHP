# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(SIGHP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Optional OpenMP support
option(USE_OPENMP "Enable OpenMP support" OFF)
if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endif()

# Set source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Include directories
include_directories(${SOURCE_DIR})

# Find MPI package
find_package(MPI REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} src/SIGHP.cpp)

# Set output directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# System-specific configurations
if(UNIX)
    add_definitions(-DUNIX_SYSTEM)
elseif(WIN32)
    add_definitions(-DWIN32_SYSTEM)
endif()

# Check for required headers
include(CheckIncludeFiles)
check_include_files(sys/mman.h HAVE_MMAN_H)
if(NOT HAVE_MMAN_H)
    message(FATAL_ERROR "Required header sys/mman.h not found")
endif()

# Add MPI compile options
add_compile_options(-O3)

# Add MPI include directories
include_directories(${MPI_INCLUDE_PATH})

# Add MPI executable
add_executable(sighp_mpi src/SIGHP_MPI.cpp)

# Link MPI libraries
target_link_libraries(sighp_mpi ${MPI_CXX_LIBRARIES})

