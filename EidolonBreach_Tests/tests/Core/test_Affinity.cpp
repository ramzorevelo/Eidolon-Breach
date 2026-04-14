/**
 * @file test_Affinity.cpp
 * @brief Unit tests for Affinity enum and conversion.
 */

#include "Core/Affinity.h"
#include "doctest.h"

TEST_CASE("Affinity: affinityToString covers all values")
{
    CHECK(affinityToString(Affinity::Blaze) == "Blaze");
    CHECK(affinityToString(Affinity::Frost) == "Frost");
    CHECK(affinityToString(Affinity::Tempest) == "Tempest");
    CHECK(affinityToString(Affinity::Terra) == "Terra");
    CHECK(affinityToString(Affinity::Aether) == "Aether");
}