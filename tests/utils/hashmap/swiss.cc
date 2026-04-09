#include <catch2/catch_all.hpp>

import std;
import hashmap;

TEST_CASE("Basic insert and find", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, std::string> map;

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    SECTION("Find inserted values") {
        CHECK(map.find(1).has_value());
        CHECK(map.find(1).value() == "one");

        CHECK(map.find(2).has_value());
        CHECK(map.find(2).value() == "two");

        CHECK(map.find(3).has_value());
        CHECK(map.find(3).value() == "three");
    }

    SECTION("Miss on non-existent key") { CHECK(!map.find(99).has_value()); }
}

TEST_CASE("Add values with a shared key", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, std::string> map;

    map.insert(1, "one");
    REQUIRE(map.size() == 1);

    map.insert(1, "ONE");
    REQUIRE(map.size() == 2);
}

TEST_CASE("Erase operation", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, std::string> map;

    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    REQUIRE(map.size() == 3);

    SECTION("Erase and verify") {
        map.erase(2);

        CHECK(!map.find(2).has_value());
        CHECK(map.find(1).has_value());
        CHECK(map.find(3).has_value());
        CHECK(map.size() == 2);
    }
}

TEST_CASE("Rehashing on large insert", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, int> map;

    for (int i = 0; i < 100; ++i) {
        map.insert(i, i * 10);
    }

    CHECK(map.size() == 100);

    SECTION("Spot checks after rehash") {
        for (int i = 0; i < 100; i += 10) {
            REQUIRE(map.find(i).has_value());
            CHECK(map.find(i).value() == i * 10);
        }
    }

    SECTION("All entries recoverable") {
        for (int i = 0; i < 100; ++i) {
            CHECK(map.find(i).value() == i * 10);
        }
    }
}

TEST_CASE("String keys", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<std::string, int> map;

    map.insert("apple", 1);
    map.insert("banana", 2);
    map.insert("cherry", 3);

    SECTION("Find string keys") {
        CHECK(map.find("apple").value() == 1);
        CHECK(map.find("banana").value() == 2);
        CHECK(map.find("cherry").value() == 3);
    }

    SECTION("String key miss") { CHECK(!map.find("date").has_value()); }
}

TEST_CASE("Empty map behavior", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, std::string> map;

    CHECK(map.empty());
    CHECK(map.size() == 0);
    CHECK(!map.find(1).has_value());
}

TEST_CASE("Collision handling", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, int> map;

    for (int i = 0; i < 50; ++i) {
        map.insert(i, i * 100);
    }

    SECTION("All entries findable after collisions") {
        for (int i = 0; i < 50; ++i) {
            REQUIRE(map.find(i).has_value());
            CHECK(map.find(i).value() == i * 100);
        }
    }
}

TEST_CASE("Erase and reinsert", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, std::string> map;

    map.insert(5, "five");
    REQUIRE(map.find(5).value() == "five");

    map.erase(5);
    CHECK(!map.find(5).has_value());

    map.insert(5, "FIVE");
    CHECK(map.find(5).value() == "FIVE");
}

TEST_CASE("Mixed operations stress test", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, int> map;

    SECTION("Insert, erase, reinsert sequence") {
        map.insert(1, 10);
        map.insert(2, 20);
        map.insert(3, 30);
        CHECK(map.size() == 3);

        // 1. Double insert key 2
        map.upsert(2, 200);
        CHECK(map.size() == 3);

        auto val = map.find(2);
        CHECK(val.has_value());
        CHECK(val.value() == 200);

        map.erase(2);
        CHECK(map.size() == 2);

        // 5. Reinsert
        map.insert(2, 2000);
        CHECK(map.find(2).value() == 2000);
        CHECK(map.size() == 3);
    }
}

TEST_CASE("Large dataset operations", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, std::string> map;

    const int N = 200;
    for (int i = 0; i < N; ++i) {
        map.insert(i, "val_" + std::to_string(i));
    }

    CHECK(map.size() == N);

    SECTION("Verify random samples") {
        CHECK(map.find(0).value() == "val_0");
        CHECK(map.find(N / 4).value() == "val_" + std::to_string(N / 4));
        CHECK(map.find(N / 2).value() == "val_" + std::to_string(N / 2));
        CHECK(map.find(N - 1).value() == "val_" + std::to_string(N - 1));
    }
}

TEST_CASE("Fingerprint edge cases", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, int> map;

    SECTION("Keys hashing to reserved fingerprints") {
        map.insert(0xFF, 1);
        map.insert(0x7E, 2);
        map.insert(0xFE, 3);

        CHECK(map.find(0xFF).value() == 1);
        CHECK(map.find(0x7E).value() == 2);
        CHECK(map.find(0xFE).value() == 3);
        CHECK(map.size() == 3);
    }
}

TEST_CASE("Multiple rehash cycles", "[SwissHashMap]") {
    hashmap::swiss::SwissHashMap<int, int> map;

    for (int i = 0; i < 300; ++i) {
        map.insert(i, i * 5);
    }

    CHECK(map.size() == 300);

    for (int i = 0; i < 300; i += 50) {
        REQUIRE(map.find(i).has_value());
        CHECK(map.find(i).value() == i * 5);
    }
}
