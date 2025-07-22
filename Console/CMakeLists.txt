cmake_minimum_required(VERSION 3.31)
project(servant-console)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# ---------------------------
# Detect OS name
# ---------------------------
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS_NAME macOS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS_NAME Linux)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OS_NAME Windows)
else ()
    set(OS_NAME Other)
endif ()

# ---------------------------
# Set output directories
# ---------------------------
set(PublishFolder "Publish")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/${PublishFolder})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/${PublishFolder})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/${OS_NAME}/${PublishFolder})

# ---------------------------
# Source files
file(GLOB SERVICES_SRC Services/*.cpp Services/*.h)

# ---------------------------
# Dependencies from vcpkg
find_package(Drogon CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ---------------------------
# Static asset copy
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

# ---------------------------
# Main executable
# ---------------------------
add_executable(servant-console main.cpp ${SERVICES_SRC})
target_include_directories(servant-console PRIVATE Services)

target_include_directories(servant-console PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(servant-console
        PRIVATE
        Drogon::Drogon
        ${SQLite3_LIBRARIES}
        CURL::libcurl
        nlohmann_json::nlohmann_json
)


add_dependencies(servant-console copy_static_files)
