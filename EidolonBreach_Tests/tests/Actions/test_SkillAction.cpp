#include "doctest.h"
#include "Actions/SkillAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include <memory>
#include "test_helpers.h"


TEST_CASE("SkillAction: requires and consumes 1 SP")
{
    Party pp, ep;
    auto heroRaw = makeHero();
    auto* heroPtr = heroRaw.get();
    pp.addUnit(std::move(heroRaw));
    ep.addUnit(makeEnemy());

    heroPtr->useSp(3);
    CHECK(heroPtr->getSp() == 0);

    SkillAction skill;
    CHECK(!skill.isAvailable(*heroPtr));

    heroPtr->gainSp(1);
    CHECK(skill.isAvailable(*heroPtr));

    TargetInfo t{ TargetInfo::Type::Enemy, 0 };
    skill.execute(*heroPtr, pp, ep, t);
    CHECK(heroPtr->getSp() == 0);
}