cmake_minimum_required(VERSION 3.28)

# Set the project name and language
project(SDL3GameOfLife LANGUAGES CXX)

# Set the output directory for build objects and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Prevent installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Get, build and add SDL to project from git repository
include(FetchContent)
FetchContent_Declare(SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SDL)

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

# Get all of the source files
file(GLOB SOURCE_FILES src/*.cpp)

# Create executable from source files
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# Set C++17 version
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17)

# Link libraries to executable
target_link_libraries(${EXECUTABLE_NAME}
PUBLIC 
	SDL3::SDL3
	# Link additional libraries if needed
)
