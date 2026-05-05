/**
 * @file test_ActionData.cpp
 * @brief Tests for ActionData defaults and CombatUtils new overload.
 */
#include "Core/ActionData.h"
#include "Core/CombatUtils.h"
#include "Core/Stats.h"
#include "doctest.h"
#include <ostream>
#include <string>

TEST_CASE("ActionData: default-constructed values are correct")
{
    ActionData data{};
    CHECK(data.skillPower == doctest::Approx(1.0f));
    CHECK(data.scaling == ScalingStat::ATK);
    CHECK(data.spCost == 0);
    CHECK(data.energyCost == 0);
    CHECK(data.energyGain == 0);
    CHECK(data.toughnessDamage == 0);
    CHECK(data.targetMode == TargetMode::SingleEnemy);
    CHECK(data.affinity == Affinity::Aether);
}

TEST_CASE("CombatUtils::calculateDamage (ActionData overload): ATK scaling")
{
    Stats attacker{100, 100, 20, 5, 10}; // ATK = 20
    Stats defender{100, 100, 10, 0, 5};  // DEF = 0
    // damage = 1.0 * 20 * (1 - 0/100) = 20
    int dmg = CombatUtils::calculateDamage(1.0f, attacker, defender, ScalingStat::ATK);
    CHECK(dmg == 20);
}

TEST_CASE("CombatUtils::calculateDamage (ActionData overload): DEF mitigation applied")
{
    Stats attacker{100, 100, 20, 5, 10};  // ATK = 20
    Stats defender{100, 100, 10, 100, 5}; // DEF = 100
    // damage = 1.0 * 20 * (1 - 100/200) = 20 * 0.5 = 10
    int dmg = CombatUtils::calculateDamage(1.0f, attacker, defender, ScalingStat::ATK);
    CHECK(dmg == 10);
}

TEST_CASE("CombatUtils::calculateDamage (ActionData overload): skill power scales damage")
{
    Stats attacker{100, 100, 20, 5, 10}; // ATK = 20
    Stats defender{100, 100, 10, 0, 5};  // DEF = 0
    // damage = 1.5 * 20 * 1.0 = 30
    int dmg = CombatUtils::calculateDamage(1.5f, attacker, defender, ScalingStat::ATK);
    CHECK(dmg == 30);
}

TEST_CASE("CombatUtils::calculateDamage (ActionData overload): minimum damage is 1")
{
    Stats attacker{100, 100, 0, 0, 5};    // ATK = 0
    Stats defender{100, 100, 10, 500, 5}; // DEF = 500
    int dmg = CombatUtils::calculateDamage(1.0f, attacker, defender, ScalingStat::ATK);
    CHECK(dmg == 1);
}

TEST_CASE("CombatUtils::calculateDamage (flat overload): unchanged from Phase 1")
{
    // DEF = 0: full damage passes through.
    CHECK(CombatUtils::calculateDamage(30, 0) == 30);
    // DEF = 100: 30 * (1 - 100/200) = 15
    CHECK(CombatUtils::calculateDamage(30, 100) == 15);
}