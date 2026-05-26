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

TEST_CASE("is_terminal(TaskStatus)") {
    using enum model::TaskStatus;
    CHECK(model::is_terminal(COMPLETED));
    CHECK(model::is_terminal(FAILED));
    CHECK(model::is_terminal(TIMED_OUT));
    CHECK(model::is_terminal(SKIPPED));
    CHECK(model::is_terminal(CANCELED));
    CHECK_FALSE(model::is_terminal(SCHEDULED));
    CHECK_FALSE(model::is_terminal(IN_PROGRESS));
}
