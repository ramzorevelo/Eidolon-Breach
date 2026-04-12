#include "doctest.h"
#include "Entities/Unit.h"
#include "Entities/PlayableCharacter.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "test_helpers.h"
#include <memory>



TEST_CASE("Unit: HP management")
{
    auto hero = makeHero();
    CHECK(hero->getHp() == 120);
    CHECK(hero->getMaxHp() == 120);
    CHECK(hero->isAlive());

    hero->takeDamage(50);
    CHECK(hero->getHp() == 70);

    hero->takeDamage(999);
    CHECK(hero->getHp() == 0);
    CHECK(!hero->isAlive());

    hero->heal(30);
    CHECK(hero->getHp() == 30);

    hero->heal(999);
    CHECK(hero->getHp() == 120);
}

TEST_CASE("Unit: isBroken returns false by default (PlayableCharacter)")
{
    auto hero = makeHero();
    CHECK(!hero->isBroken());
    hero->applyToughnessHit(9999);
    CHECK(!hero->isBroken());
}