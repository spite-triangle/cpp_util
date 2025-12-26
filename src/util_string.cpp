#include "doctest.h"

#include "util.hpp"

using namespace util;

TEST_CASE("format") {
    CHECK(format("%d + %d = %d", 1, 2, 3) == "1 + 2 = 3");
}

TEST_CASE("toLower and toUpper") {
    CHECK(toLower("ABCdEf") == "abcdef");
    CHECK(toUpper("abcDef") == "ABCDEF");
}

TEST_CASE("startWith/endWith/contain") {
    CHECK(startWith("abcdef", "abc"));
    CHECK_FALSE(startWith("abcdef", "def"));
    CHECK(endWith("abcdef", "def"));
    CHECK_FALSE(endWith("abcdef", "abc"));
    CHECK(contain("abcdef", "cd"));
    CHECK_FALSE(contain("abcdef", "gh"));
}

TEST_CASE("split_regex") {
    std::regex re_comma(",");
    auto v = split("a,b,c", re_comma);
    CHECK(v.size() == 3);
    CHECK(v[0] == "a");
    CHECK(v[1] == "b");
    CHECK(v[2] == "c");
}

TEST_CASE("repleace") {
    CHECK(repleace("aabbcc", "bb", "dd") == "aaddcc");
    CHECK(repleace("aabbccbb", "bb", "dd", true) == "aaddccdd");
}

TEST_CASE("split") {
    auto v = split("a,b,c", ",");
    CHECK(v.size() == 3);
    CHECK(v[0] == "a");
    CHECK(v[1] == "b");
    CHECK(v[2] == "c");
}

TEST_CASE("trim") {
    CHECK(ltrim("   abc") == "abc");
    CHECK(rtrim("abc   ") == "abc");
    CHECK(trims("  abc  ") == "abc");
}

TEST_CASE("startWith/endWith/contain_regex") {
    std::regex re_start("abc");
    std::regex re_end("def");
    std::regex re_contain("cd");
    CHECK(startWith("abcdef", re_start, true));
    CHECK_FALSE(startWith("abcdef", std::regex("def"), true));
    CHECK(endWith("abcdef", re_end, true));
    CHECK_FALSE(endWith("abcdef", std::regex("abc"), true));
    CHECK(contain("abcdef", re_contain));
    CHECK_FALSE(contain("abcdef", std::regex("gh")));
}

TEST_CASE("repleace_regex") {
    std::regex re_b("b+");
    CHECK(repleace("aabbbcc", re_b, "d", true) == "aadcc");
    CHECK(repleace("aabbbcc", re_b, "d", false) == "aadcc"); // only first
}

TEST_CASE("toNumber") {
    int i = 0;
    double d = 0.0;
    size_t sz = 0;
    float f = 0.0f;
    long long ll = 0;
    unsigned long long ull = 0;

    CHECK(toNumber("123", i));
    CHECK(i == 123);
    CHECK(toNumber("3.14", d));
    CHECK(d == doctest::Approx(3.14));
    CHECK(toNumber("456", sz));
    CHECK(sz == 456);
    CHECK(toNumber("2.718", f));
    CHECK(f == doctest::Approx(2.718f));
    CHECK(toNumber("9223372036854775807", ll));
    CHECK(ll == 9223372036854775807LL);
    CHECK(toNumber("18446744073709551615", ull));
    CHECK(ull == 18446744073709551615ULL);

    // 错误输入
    int invalid = 42;
    CHECK_FALSE(toNumber("abc", invalid));
    CHECK(invalid == 42); // 保持原值
    double invalid_d = 1.23;
    CHECK_FALSE(toNumber("not_a_number", invalid_d));
    CHECK(invalid_d == 1.23);
}
