cmake_minimum_required(VERSION 3.15)
project(Servants)

set(CMAKE_CXX_STANDARD 17)

find_package(Drogon REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Catch2 REQUIRED)

add_executable(Servants
    main.cpp
    server.cpp
    db.cpp
    service_main.cpp
)

target_link_libraries(Servants
    Drogon::Drogon
    SQLite::SQLite3
    ${PostgreSQL_LIBRARIES}
)

enable_testing()
add_executable(UnitTests tests/test_main.cpp)
target_link_libraries(UnitTests Catch2::Catch2WithMain)
add_test(NAME MyTest COMMAND UnitTests)
