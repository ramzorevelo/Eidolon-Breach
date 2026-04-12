#include "doctest.h"
#include "Actions/BasicStrikeAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Entities/IAIStrategy.h"
#include <memory>
#include "test_helpers.h"


TEST_CASE("BasicStrikeAction: deals damage and toughness, grants SP and Energy")
{
    Party pp, ep;
    auto heroRaw = makeHero();
    auto* heroPtr = heroRaw.get();
    pp.addUnit(std::move(heroRaw));

    auto enemyRaw = makeEnemy(100, 50);
    auto* enemyPtr = enemyRaw.get();
    ep.addUnit(std::move(enemyRaw));

    TargetInfo t{ TargetInfo::Type::Enemy, 0 };
    BasicStrikeAction action;
    ActionResult result = action.execute(*heroPtr, pp, ep, t);

    CHECK(result.type == ActionResult::Type::Damage);
    CHECK(result.value == 15);
    CHECK(enemyPtr->getHp() == 85);
    CHECK(enemyPtr->getToughness() == 40);
    CHECK(heroPtr->getSp() == 4);
    CHECK(heroPtr->getEnergy() == 20);
}

TEST_CASE("BasicStrikeAction: DEF reduction formula")
{
    Party pp, ep;
    auto heroRaw = makeHero();
    auto* heroPtr = heroRaw.get();
    pp.addUnit(std::move(heroRaw));

    auto enemyRaw = std::make_unique<Enemy>(
        "e", "E", Stats{ 100,100,10,100,5 }, Affinity::Terra, 50,
        std::make_unique<BasicAIStrategy>()
        );
    auto* enemyPtr = enemyRaw.get();
    ep.addUnit(std::move(enemyRaw));

    TargetInfo t{ TargetInfo::Type::Enemy, 0 };
    BasicStrikeAction action;
    ActionResult result = action.execute(*heroPtr, pp, ep, t);

    CHECK(result.value == 7);
    CHECK(enemyPtr->getHp() == 93);
}