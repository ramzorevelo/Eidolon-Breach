#include "doctest.h"
#include "Entities/PlayableCharacter.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include <memory>
#include "test_helpers.h"


TEST_CASE("PlayableCharacter: SP and Energy management")
{
    auto hero = makeHero();
    CHECK(hero->getSp() == 3);
    CHECK(hero->getEnergy() == 0);
    CHECK(!hero->ultimateReady());

    hero->gainEnergy(60);
    CHECK(hero->getEnergy() == 60);

    hero->gainEnergy(60);
    CHECK(hero->getEnergy() == 100);
    CHECK(hero->ultimateReady());

    hero->resetEnergy();
    CHECK(hero->getEnergy() == 0);

    hero->gainSp(3);
    CHECK(hero->getSp() == 5);

    hero->useSp(2);
    CHECK(hero->getSp() == 3);
}