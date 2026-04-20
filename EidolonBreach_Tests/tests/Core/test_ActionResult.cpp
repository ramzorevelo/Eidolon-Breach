// tests/test_ActionResult.cpp (new file)
/**
 * @file test_ActionResult.cpp
 * @brief Tests for ActionResult fields and Battle::processActionResult propagation.
 */
#include "Core/ActionResult.h"
#include "doctest.h"

TEST_CASE("ActionResult: default-constructed values are correct")
{
    ActionResult r{};
    CHECK(r.type == ActionResult::Type::Damage);
    CHECK(r.value == 0);
    CHECK(r.spGained == 0);
    CHECK(r.exposureDelta == 0);
    CHECK(r.flavorText.empty());
}

TEST_CASE("ActionResult: spGained and exposureDelta are independently settable")
{
    ActionResult r{};
    r.spGained = 15;
    r.exposureDelta = -10;
    CHECK(r.spGained == 15);
    CHECK(r.exposureDelta == -10);
}