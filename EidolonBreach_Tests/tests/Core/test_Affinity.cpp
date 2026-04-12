#include "doctest.h"
#include "Core/Affinity.h"

TEST_CASE("Affinity: affinityToString covers all values")
{
    CHECK(affinityToString(Affinity::Blaze) == "Blaze");
    CHECK(affinityToString(Affinity::Frost) == "Frost");
    CHECK(affinityToString(Affinity::Tempest) == "Tempest");
    CHECK(affinityToString(Affinity::Terra) == "Terra");
    CHECK(affinityToString(Affinity::Aether) == "Aether");
}