#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

TEST_CASE("Basic Check") {
    REQUIRE(1 + 1 == 2);
}
