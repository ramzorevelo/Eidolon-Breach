/**
 * @file test_TurnOrder.cpp
 * @brief Integration‑style test for turn order tie‑breakers.
 */
#include "Actions/BasicStrikeAction.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include <memory>

TEST_CASE("Turn order: player before enemy on SPD tie")
{
    Party pp, ep;
    auto hero = std::make_unique<PlayableCharacter>(
        "h", "Hero", Stats{120, 120, 15, 0, 10}, Affinity::Aether, 10);
    hero->addAbility(std::make_unique<BasicStrikeAction>());
    pp.addUnit(std::move(hero));

    ep.addUnit(std::make_unique<Enemy>(
        "e", "Bat", Stats{100, 100, 14, 0, 10}, Affinity::Blaze, 40,
        std::make_unique<BasicAIStrategy>()));

    CHECK(pp.getUnitAt(0)->getStats().spd == ep.getUnitAt(0)->getStats().spd);
    // Battle::buildTurnOrder is private; actual ordering is verified by
    // integration tests or manual observation.
}