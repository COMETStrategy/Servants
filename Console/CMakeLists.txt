cmake_minimum_required(VERSION 3.31)
project(servant-console)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# ---------------------------
# Detect OS name
# ---------------------------
set(OPENSSL_SSL_LIBRARY "OpenSSL::SSL")
set(OPENSSL_CRYPTO_LIBRARY "OpenSSL::Crypto")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS_NAME macOS)
    set(SQLITE3_LIBRARY_PATH "/opt/homebrew/opt/sqlite/lib/libsqlite3.3.50.3.dylib") 
    set(OPENSSL_SSL_LIBRARY "/opt/homebrew/opt/openssl/lib/libssl.dylib")
    set(OPENSSL_CRYPTO_LIBRARY "/opt/homebrew/opt/openssl/lib/libcrypto.dylib")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS_NAME Linux)
    set(SQLITE3_LIBRARY_PATH "/usr/lib/x86_64-linux-gnu/libsqlite3.so") # or as appropriate
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OS_NAME Windows)
    set(SQLITE3_LIBRARY_PATH "C:/Program Files/vcpkg/installed/x64-windows/debug/lib/sqlite3.lib")
else ()
    set(OS_NAME Other)
    set(SQLITE3_LIBRARY_PATH "sqlite3") # fallback, may not work
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
#find_package(SQLite3 REQUIRED) #included in Dragon
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

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

target_link_libraries(servant-console
        PRIVATE
        Drogon::Drogon
        CURL::libcurl
        nlohmann_json::nlohmann_json
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${SQLITE3_LIBRARY_PATH}
)


add_dependencies(servant-console copy_static_files)
