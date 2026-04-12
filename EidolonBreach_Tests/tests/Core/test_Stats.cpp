#include "doctest.h"
#include "Core/Stats.h"

TEST_CASE("Stats: default-constructed values are zero")
{
    Stats s{};
    CHECK(s.hp == 0);
    CHECK(s.maxHp == 0);
    CHECK(s.atk == 0);
    CHECK(s.def == 0);
    CHECK(s.spd == 0);
}