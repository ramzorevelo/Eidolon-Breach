/**
 * @file test_FixedDungeon.cpp
 * @brief Smoke tests for DungeonTable fixed layouts.
 */
#include "Dungeon/DungeonTable.h"
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

TEST_CASE("DungeonTable: Dungeon 7 ends with boss")
{
    const auto &d7{DungeonTable::getClassicDungeons().back()};
    CHECK(d7.fixedLayout.back() == "boss");
}

TEST_CASE("DungeonTable: no dungeon before dungeon 7 contains a boss node")
{
    const auto &dungeons{DungeonTable::getClassicDungeons()};
    for (std::size_t i{0}; i + 1 < dungeons.size(); ++i)
    {
        for (const auto &nodeType : dungeons[i].fixedLayout)
            CHECK(nodeType != "boss");
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