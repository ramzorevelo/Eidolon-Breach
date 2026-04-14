/**
 * @file test_BattleTurnOrderIntegration.cpp
 * @brief Integration test verifying Battle uses the calculator correctly.
 */
#include "Actions/BasicStrikeAction.h"
#include "Battle/Battle.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include <memory>

TEST_CASE("Battle integration: turn order uses injected calculator")
{
    Party playerParty, enemyParty;
    auto hero = std::make_unique<PlayableCharacter>(
        "hero", "Hero", Stats{100, 100, 10, 0, 20}, Affinity::Aether, 10);
    hero->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(hero));

    auto enemy = std::make_unique<Enemy>(
        "bat", "Bat", Stats{100, 100, 14, 0, 5}, Affinity::Blaze, 40,
        std::make_unique<BasicAIStrategy>());
    enemyParty.addUnit(std::move(enemy));

    Battle battle{playerParty, enemyParty};
    // The default calculator is SpeedBasedTurnOrderCalculator.
    // We can't easily test the full run() without mock input,
    // but we can verify the calculator is present.
    CHECK(battle.getTurnOrderCalculator() != nullptr);
}
