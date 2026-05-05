/**
 * @file test_CharFlag.cpp
 * @brief Tests for the CharFlag bitmask on PlayableCharacter.
 */
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("CharFlag: all flags start cleared")
{
    auto hero = makeHero();
    CHECK(!hero->isArchSkillUnlocked());
    CHECK(!hero->isResonatingProcArmed());
    CHECK(!hero->isSurgingProcArmed());
    CHECK(!hero->isBreachbornActive());
    CHECK(!hero->isFractured());
    CHECK(hero->canUseConsumable()); // ConsumableUsedThisBattle starts false
}

TEST_CASE("CharFlag: setting one flag does not affect others")
{
    auto hero = makeHero();
    hero->armResonatingProc();
    CHECK(hero->isResonatingProcArmed());
    CHECK(!hero->isSurgingProcArmed());
    CHECK(!hero->isFractured());
    CHECK(!hero->isBreachbornActive());
}

TEST_CASE("CharFlag: consumeResonatingProc clears only ResonatingProcArmed")
{
    auto hero = makeHero();
    hero->armResonatingProc();
    hero->armSurgingProc();
    hero->consumeResonatingProc();
    CHECK(!hero->isResonatingProcArmed());
    CHECK(hero->isSurgingProcArmed()); // unaffected
}

TEST_CASE("CharFlag: resetBattleConsumableState clears per-battle flags, preserves permanent")
{
    auto hero = makeHero();
    hero->armResonatingProc();
    hero->armSurgingProc();
    hero->applyFracture();  // permanent — must survive reset
    hero->applyUnlocks(20); // ArchSkillUnlocked — must survive reset

    hero->resetBattleConsumableState();

    CHECK(!hero->isResonatingProcArmed());
    CHECK(!hero->isSurgingProcArmed());
    CHECK(hero->isFractured());         // preserved
    CHECK(hero->isArchSkillUnlocked()); // preserved
}

TEST_CASE("CharFlag: applyFracture is idempotent")
{
    auto hero = makeHero();
    hero->applyFracture();
    hero->applyFracture(); // second call must not corrupt other bits
    CHECK(hero->isFractured());
    CHECK(!hero->isResonatingProcArmed());
}