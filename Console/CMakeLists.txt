cmake_minimum_required(VERSION 3.31)
project(servant-console)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")


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
# set($PublishFolder, $<CONFIG>)
set($PublishFolder "Publish")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/${PublishFolder})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/${PublishFolder})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/${PublishFolder})

# Source files
file(GLOB SERVICES_SRC Services/*.cpp  Services/*.h)

# Find and link libcurl
find_package(CURL REQUIRED)

file(GLOB_RECURSE STATIC_FILES ${CMAKE_SOURCE_DIR}/static/*)

add_custom_command(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/static
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/static
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/static
        DEPENDS ${STATIC_FILES}
        COMMENT "Copying static files to runtime directory"
)

add_custom_target(copy_static_files ALL
        DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/static
        COMMENT "Copying static files to runtime directory"
)


# Main executable
add_executable(servant-console main.cpp ${SERVICES_SRC}
        Services/Servant.cpp
        Services/Servant.h
        Services/Scheduler.cpp
        Services/Scheduler.h)
target_include_directories(servant-console PRIVATE Services)
target_link_libraries(servant-console PRIVATE drogon sqlite3 CURL::libcurl)

# Ensure copy_static_files and copy_view_files runs before servant-console
add_dependencies(servant-console copy_static_files)

# Unit tests
enable_testing()
file(GLOB SERVICES_SRC Services/*.cpp) # Include all .cpp files in the Services folder
add_executable(servant-console-tests GTests/LoggerTest.cpp ${SERVICES_SRC}
        Services/Servant.cpp
        Services/Servant.h
        Services/Scheduler.cpp
        Services/Scheduler.h)
target_link_libraries(servant-console-tests PRIVATE gtest gtest_main drogon sqlite3 CURL::libcurl)
target_include_directories(servant-console-tests PRIVATE Services)
add_test(NAME servant-console-tests COMMAND servant-console-tests)
