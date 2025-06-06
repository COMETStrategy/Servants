cmake_minimum_required(VERSION 3.31)
project(servant-console)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

# Fetch Drogon
FetchContent_Declare(
        drogon
        GIT_REPOSITORY https://github.com/drogonframework/drogon.git
        GIT_TAG master
)
FetchContent_MakeAvailable(drogon)

# Fetch Google Test
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Detect OS
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(OS_NAME macOS)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(OS_NAME Linux)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(OS_NAME Windows)
else ()
    set(OS_NAME Other)
endif ()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/$<CONFIG>)

# Source files
file(GLOB SERVICES_SRC Services/*.cpp  Services/*.h)

# Main executable
add_executable(servant-console main.cpp ${SERVICES_SRC})
target_include_directories(servant-console PRIVATE Services)
target_link_libraries(servant-console PRIVATE drogon sqlite3)

# Unit tests
enable_testing()
file(GLOB SERVICES_SRC Services/*.cpp) # Include all .cpp files in the Services folder
add_executable(servant-console-tests GTests/LoggerTest.cpp ${SERVICES_SRC} )
target_link_libraries(servant-console-tests PRIVATE gtest gtest_main drogon sqlite3)
target_include_directories(servant-console-tests PRIVATE Services)
add_test(NAME servant-console-tests COMMAND servant-console-tests)