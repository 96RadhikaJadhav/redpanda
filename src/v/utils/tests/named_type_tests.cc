#define BOOST_TEST_MODULE utils

#include "seastarx.h"
#include "utils/named_type.h"

#include <seastar/core/sstring.hh>

#include <boost/test/unit_test.hpp>

#include <set>

BOOST_AUTO_TEST_CASE(named_type_basic) {
    using int_alias = named_type<int32_t, struct int_alias_test_module>;
    constexpr auto x = int_alias(5);
    BOOST_REQUIRE(x == 5);
    BOOST_REQUIRE(x <= 5);
    BOOST_REQUIRE(x < 6);
    BOOST_REQUIRE(x != 50);
    BOOST_REQUIRE(x > 4);
    BOOST_REQUIRE(x >= 5);
}
BOOST_AUTO_TEST_CASE(named_type_set) {
    using int_alias = named_type<int32_t, struct int_alias_test_module>;
    std::set<int_alias> foo;
    for (int32_t i = 0; i < 100; ++i) {
        foo.insert(i);
        BOOST_REQUIRE(foo.find(i) != foo.end());
    }
}

BOOST_AUTO_TEST_CASE(named_type_unordered_map) {
    using int_alias = named_type<int32_t, struct int_alias_test_module>;
    std::unordered_map<int_alias, int_alias> foo;
    for (int32_t i = 0; i < 100; ++i) {
        foo[i] = i;
    }
    BOOST_REQUIRE(foo[5] != 4);
}

BOOST_AUTO_TEST_CASE(string_named_type_basic) {
    using string_alias = named_type<sstring, struct sstring_alias_test_module>;
    string_alias x;
    x = sstring("foobar");
    BOOST_REQUIRE(x == sstring("foobar"));
}

BOOST_AUTO_TEST_CASE(named_type_string_set) {
    using string_alias = named_type<sstring, struct sstring_alias_test_module>;
    std::set<string_alias> foo;
    for (int32_t i = 0; i < 10; ++i) {
        sstring x = to_sstring(i);
        foo.insert(x);
        BOOST_REQUIRE(foo.find(x) != foo.end());
    }
}