#include "doctest.h"
#include "Actions/UltimateAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include <memory>
#include "test_helpers.h"


TEST_CASE("UltimateAction: requires full energy, resets it and grants 2 SP")
{
    Party pp, ep;
    auto heroRaw = makeHero();
    auto* heroPtr = heroRaw.get();
    pp.addUnit(std::move(heroRaw));
    ep.addUnit(makeEnemy());

    UltimateAction ult;
    CHECK(!ult.isAvailable(*heroPtr));

    heroPtr->gainEnergy(100);
    CHECK(ult.isAvailable(*heroPtr));

    TargetInfo t{ TargetInfo::Type::Enemy, 0 };
    ult.execute(*heroPtr, pp, ep, t);

    CHECK(heroPtr->getEnergy() == 0);
    CHECK(heroPtr->getSp() == 5);
}