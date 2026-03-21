#include "doctest.h"

#include "utils/all"

TEST_CASE("optional"){
    util::optional<int> opt1;
    CHECK(opt1.has_value() == false);
    CHECK(opt1 == util::nullopt);

    opt1.emplace(1);
    CHECK(opt1.has_value() == true);

    util::optional<int> opt2(1);
    CHECK(opt1 == opt2);
}