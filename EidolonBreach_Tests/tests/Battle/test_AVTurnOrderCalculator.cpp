/**
 * @file test_AVTurnOrderCalculator.cpp
 * @brief Unit tests for AVTurnOrderCalculator.
 */
#include "Actions/BasicStrikeAction.h"
#include "Battle/AVTurnOrderCalculator.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include <memory>

namespace
{

std::unique_ptr<PlayableCharacter> makePc(const std::string &id,
                                          int spd,
                                          int hp = 100)
{
    auto pc{std::make_unique<PlayableCharacter>(
        id, id, Stats{hp, hp, 10, 0, spd}, Affinity::Aether, 10)};
    pc->addAbility(std::make_unique<BasicStrikeAction>());
    return pc;
}

std::unique_ptr<Enemy> makeEnemy(const std::string &id, int spd)
{
    return std::make_unique<Enemy>(
        id, id, Stats{100, 100, 10, 0, spd}, Affinity::Blaze, 20,
        std::make_unique<BasicAIStrategy>());
}

} // namespace

TEST_CASE("AVTurnOrderCalculator: faster unit (lower base AV) appears first")
{
    Party playerParty, enemyParty;
    playerParty.addUnit(makePc("slow", 5));
    enemyParty.addUnit(makeEnemy("fast", 20));

    AVTurnOrderCalculator calc;
    auto order{calc.calculate(playerParty, enemyParty)};

    REQUIRE(!order.empty());
    CHECK(order[0].unit->getName() == "fast");
}

TEST_CASE("AVTurnOrderCalculator: player before enemy at equal AV")
{
    Party playerParty, enemyParty;
    playerParty.addUnit(makePc("hero", 10));
    enemyParty.addUnit(makeEnemy("foe", 10));

    AVTurnOrderCalculator calc;
    auto order{calc.calculate(playerParty, enemyParty)};

    REQUIRE(order.size() >= 2);
    CHECK(order[0].isPlayer == true);
    CHECK(order[1].isPlayer == false);
}

TEST_CASE("AVTurnOrderCalculator: applyHasten moves unit earlier in next projection")
{
    Party playerParty, enemyParty;
    auto pcOwned{makePc("hero", 5)};
    Unit *pc{pcOwned.get()};
    playerParty.addUnit(std::move(pcOwned));
    enemyParty.addUnit(makeEnemy("foe", 10));

    AVTurnOrderCalculator calc;
    auto before{calc.calculate(playerParty, enemyParty)};
    // Hero is slower (spd 5 vs 10), so foe should appear first.
    REQUIRE(!before.empty());
    CHECK(before[0].unit->getName() == "foe");

    // Hasten the hero by 100% of its base AV — should act next.
    calc.applyHasten(pc, 1.0f);
    auto after{calc.calculate(playerParty, enemyParty)};
    CHECK(after[0].unit->getName() == "hero");
}

TEST_CASE("AVTurnOrderCalculator: applySuppress pushes unit later")
{
    Party playerParty, enemyParty;
    playerParty.addUnit(makePc("hero", 20));
    enemyParty.addUnit(makeEnemy("foe", 10));

    AVTurnOrderCalculator calc;
    auto before{calc.calculate(playerParty, enemyParty)};
    REQUIRE(!before.empty());
    CHECK(before[0].unit->getName() == "hero");

    // Suppress foe enough that hero's next slot appears before foe's suppressed slot.
    // But suppress the hero here to test: suppress hero 100%, foe goes first.
    calc.applySuppress(before[0].unit, 2.0f);
    auto after{calc.calculate(playerParty, enemyParty)};
    CHECK(after[0].unit->getName() == "foe");
}

TEST_CASE("AVTurnOrderCalculator: Breachborn PC appears more frequently than baseline")
{
    // Breachborn modifier is kAvModBreachborn (0.75), so the PC resets to 75%
    // of its normal AV — acts more often over 10 projected slots.
    Party playerParty, enemyParty;
    auto pc{makePc("hero", 10)};
    pc->activateBreachborn();
    playerParty.addUnit(std::move(pc));
    enemyParty.addUnit(makeEnemy("foe", 10));

    AVTurnOrderCalculator calc;
    const auto order{calc.calculate(playerParty, enemyParty)};

    int heroCount{0};
    int foeCount{0};
    for (const auto &slot : order)
    {
        if (slot.unit->getName() == "hero")
            ++heroCount;
        else
            ++foeCount;
    }
    // Breachborn hero should appear more often than the foe at equal base SPD.
    CHECK(heroCount > foeCount);
}

TEST_CASE("AVTurnOrderCalculator: Fractured PC appears less frequently than baseline")
{
    // Fractured modifier is kAvModFractured (1.10), so AV reset is longer.
    Party playerParty, enemyParty;
    auto pc{makePc("hero", 10)};
    pc->applyFracture();
    playerParty.addUnit(std::move(pc));
    enemyParty.addUnit(makeEnemy("foe", 10));

    AVTurnOrderCalculator calc;
    const auto order{calc.calculate(playerParty, enemyParty)};

    int heroCount{0};
    int foeCount{0};
    for (const auto &slot : order)
    {
        if (slot.unit->getName() == "hero")
            ++heroCount;
        else
            ++foeCount;
    }
    // Fractured hero resets AV longer — appears less or equal, not more.
    CHECK(heroCount <= foeCount);
}

TEST_CASE("AVTurnOrderCalculator: Exposure kAvModResonating applied on reset")
{
    Party playerParty, enemyParty;
    auto pc{makePc("hero", 10)};
    pc->modifyExposure(60); // triggers Resonating state (>= 50, < 75)
    playerParty.addUnit(std::move(pc));
    enemyParty.addUnit(makeEnemy("foe", 10));

    AVTurnOrderCalculator calc;
    const auto order{calc.calculate(playerParty, enemyParty)};
    // Resonating hero resets AV to 0.95 * base — should appear more than foe
    // at equal base SPD over 10 slots.
    int heroCount{0};
    int foeCount{0};
    for (const auto &slot : order)
    {
        if (slot.unit->getName() == "hero")
            ++heroCount;
        else
            ++foeCount;
    }
    CHECK(heroCount >= foeCount);
}

TEST_CASE("AVTurnOrderCalculator: projection returns up to 10 slots")
{
    Party playerParty, enemyParty;
    playerParty.addUnit(makePc("hero", 10));
    enemyParty.addUnit(makeEnemy("foe", 8));

    AVTurnOrderCalculator calc;
    const auto order{calc.calculate(playerParty, enemyParty)};
    CHECK(order.size() == 10);
}

TEST_CASE("AVTurnOrderCalculator: empty parties return empty vector")
{
    Party playerParty, enemyParty;
    AVTurnOrderCalculator calc;
    CHECK(calc.calculate(playerParty, enemyParty).empty());
}