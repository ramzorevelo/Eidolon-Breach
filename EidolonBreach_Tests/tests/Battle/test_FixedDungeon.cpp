/**
 * @file test_FixedDungeon.cpp
 * @brief Smoke tests for DungeonTable fixed layouts.
 */
#include "Dungeon/DungeonTable.h"
#include <array>
#include <algorithm>
#include "doctest.h"

TEST_CASE("DungeonTable: all Classic dungeons have fixedLayout non-empty")
{
    for (const auto &def : DungeonTable::getClassicDungeons())
    {
        CHECK(!def.fixedLayout.empty());
        // numFloors must match layout size so the display is accurate.
        CHECK(def.numFloors == static_cast<int>(def.fixedLayout.size()));
    }
}

TEST_CASE("DungeonTable: Dungeon 1 is exactly 1 battle floor")
{
    const auto &d1{DungeonTable::getClassicDungeons().front()};
    CHECK(d1.fixedLayout.size() == 1);
    CHECK(d1.fixedLayout[0] == "battle");
}

TEST_CASE("DungeonTable: Dungeon 10 ends with boss")
{
    const auto &d10{DungeonTable::getClassicDungeons().back()};
    CHECK(d10.fixedLayout.back() == "boss");
}

TEST_CASE("DungeonTable: only chapter-boss dungeons contain a boss node")
{
    // Chapter bosses are at 0-based indices 2, 5, 8, and 9.
    const std::array<std::size_t, 4> kBossIndices{2, 5, 8, 9};
    const auto &dungeons{DungeonTable::getClassicDungeons()};
    REQUIRE(dungeons.size() == 10);
    for (std::size_t i{0}; i < dungeons.size(); ++i)
    {
        const bool shouldHaveBoss{
            std::find(kBossIndices.begin(), kBossIndices.end(), i) !=
            kBossIndices.end()};
        bool hasBoss{false};
        for (const auto &nodeType : dungeons[i].fixedLayout)
            if (nodeType == "boss")
                hasBoss = true;
        CHECK(hasBoss == shouldHaveBoss);
    }
}

TEST_CASE("DungeonTable: Dungeon 3 is the first to contain a rest node")
{
    const auto &dungeons{DungeonTable::getClassicDungeons()};
    CHECK(dungeons.size() >= 3);

    // Dungeons 1 and 2 must not have rest.
    for (const auto &nodeType : dungeons[0].fixedLayout)
        CHECK(nodeType != "rest");
    for (const auto &nodeType : dungeons[1].fixedLayout)
        CHECK(nodeType != "rest");

    // Dungeon 3 must have at least one rest.
    bool d3HasRest{false};
    for (const auto &nodeType : dungeons[2].fixedLayout)
        if (nodeType == "rest")
            d3HasRest = true;
    CHECK(d3HasRest);
}