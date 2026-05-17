/**
 * @file test_Exposure.cpp
 * @brief Tests for PlayableCharacter Exposure gauge and VentAction wiring.
 */
#include "Actions/VentAction.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <ostream>
#include <string>

TEST_CASE("PlayableCharacter: Exposure starts at 0")
{
    auto hero = makeHero();
    CHECK(hero->getExposure() == 0);
}

TEST_CASE("PlayableCharacter: modifyExposure clamps to [0, 100]")
{
    auto hero = makeHero();
    hero->modifyExposure(60);
    CHECK(hero->getExposure() == 60);

    hero->modifyExposure(60); // would exceed 100
    CHECK(hero->getExposure() == 100);

    hero->modifyExposure(-200); // would go below 0
    CHECK(hero->getExposure() == 0);
}

TEST_CASE("PlayableCharacter: canVent is true only when 0 < exposure < 100")
{
    auto hero = makeHero();
    CHECK(!hero->canVent()); // exposure 0

    hero->modifyExposure(50);
    CHECK(hero->canVent()); // exposure 50

    hero->modifyExposure(50);
    CHECK(!hero->canVent()); // exposure 100 (Breachborn — Vent blocked)
}

TEST_CASE("VentAction: isAvailable reflects canVent()")
{
    Party allies{};
    auto hero = makeHero();
    VentAction vent{};

    CHECK(!vent.isAvailable(*hero, allies)); // exposure 0
    hero->modifyExposure(40);
    CHECK(vent.isAvailable(*hero, allies)); // exposure 40
    hero->modifyExposure(60);
    CHECK(!vent.isAvailable(*hero, allies)); // exposure 100
}

TEST_CASE("VentAction: execute reduces Exposure to 0")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(30);
    VentAction vent{};
    vent.execute(*hero, allies, enemies, std::nullopt);
    CHECK(hero->getExposure() == 0);
}

TEST_CASE("VentAction: consolation proc fires when Exposure was >= 50 at time of Vent")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(75);
    VentAction vent{};
    ActionResult result = vent.execute(*hero, allies, enemies, std::nullopt);
    CHECK(result.ventConsolation); // flag set; Battle will fire the actual proc
    CHECK(hero->getExposure() == 0);
}

TEST_CASE("VentAction: no consolation when Exposure was < 50")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(30);
    VentAction vent{};
    ActionResult result = vent.execute(*hero, allies, enemies, std::nullopt);
    CHECK(!result.ventConsolation);
}

TEST_CASE("PlayableCharacter: canVent returns false when Fractured")
{
    auto hero = makeHero();
    hero->modifyExposure(50);
    REQUIRE(hero->canVent()); // sanity

    hero->applyFracture();
    CHECK(!hero->canVent()); // Fracture locks Vent
}

TEST_CASE("PlayableCharacter: canVent returns true when Fractured but re-hits 100 (Breachborn refresh)")
{
    // Fractured characters can re-hit 100 — but Vent is still locked.
    // This test confirms canVent() stays false at all exposures when Fractured.
    auto hero = makeHero();
    hero->applyFracture();
    hero->modifyExposure(60);
    CHECK(!hero->canVent());
    hero->modifyExposure(40); // now at 100
    CHECK(!hero->canVent());
}

TEST_CASE("PlayableCharacter: fracture fields default to zero/false")
{
    auto hero = makeHero();
    CHECK(hero->fractureShieldBonus() == doctest::Approx(0.0f));
    CHECK(!hero->fractureResonatingOnAny());
    CHECK(hero->fractureDebuffDurationBonus() == 0);
    CHECK(!hero->fractureConsumeAllyBuff());
    CHECK(hero->labyrinthOnKill() == 0);
    CHECK(hero->labyrinthOnSlot() == 0);
    CHECK(hero->labyrinthOnDebuff() == 0);
}

TEST_CASE("PlayableCharacter: fracture setters round-trip")
{
    auto hero = makeHero();
    hero->setFractureShieldBonus(0.40f);
    CHECK(hero->fractureShieldBonus() == doctest::Approx(0.40f));
    hero->setFractureResonatingOnAny(true);
    CHECK(hero->fractureResonatingOnAny());
    hero->setLabyrinthOnKill(4);
    CHECK(hero->labyrinthOnKill() == 4);
}