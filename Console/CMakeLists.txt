cmake_minimum_required(VERSION 3.31)
project(servants)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

# Fetch Drogon
FetchContent_Declare(
        drogon
        GIT_REPOSITORY https://github.com/drogonframework/drogon.git
        GIT_TAG master
)
FetchContent_MakeAvailable(drogon)

# Detect OS
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(OS_NAME macOS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(OS_NAME Linux)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(OS_NAME Windows)
else()
    set(OS_NAME Other)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/$<CONFIG>)

add_executable(servants main.cpp)
target_link_libraries(servants PRIVATE drogon)