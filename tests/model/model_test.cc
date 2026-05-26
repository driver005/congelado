#define UUID_SYSTEM_GENERATOR
#include <uuid.h>
#include <catch2/catch_test_macros.hpp>
import model;

TEST_CASE("generate_id produces non-nil UUIDs") {
    auto id1 = model::generate_id();
    auto id2 = model::generate_id();
    CHECK_FALSE(id1.is_nil());
    CHECK_FALSE(id2.is_nil());
    CHECK(id1 != id2);
}
