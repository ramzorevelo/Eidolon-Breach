#include "Dungeon/DungeonTable.h"
#include "doctest.h"

TEST_CASE("DungeonTable: exactly 10 Classic dungeons")
{
    CHECK(DungeonTable::getClassicDungeons().size() == 10);
}

TEST_CASE("DungeonTable: chapter assignments are 1-4 in order")
{
    const auto &d{DungeonTable::getClassicDungeons()};
    CHECK(d[0].chapter == 1);
    CHECK(d[2].chapter == 1);
    CHECK(d[3].chapter == 2);
    CHECK(d[5].chapter == 2);
    CHECK(d[6].chapter == 3);
    CHECK(d[8].chapter == 3);
    CHECK(d[9].chapter == 4);
}

TEST_CASE("DungeonTable: fixedEnemyGroups size matches fixedLayout")
{
    for (const auto &d : DungeonTable::getClassicDungeons())
        if (!d.fixedEnemyGroups.empty())
            CHECK(d.fixedEnemyGroups.size() == d.fixedLayout.size());
}

TEST_CASE("DungeonTable: first node is never rest or boss")
{
    for (const auto &d : DungeonTable::getClassicDungeons())
    {
        REQUIRE(!d.fixedLayout.empty());
        CHECK(d.fixedLayout.front() != "rest");
        CHECK(d.fixedLayout.front() != "boss");
    }
}

TEST_CASE("DungeonTable: no two rest nodes in same dungeon")
{
    for (const auto &d : DungeonTable::getClassicDungeons())
    {
        int restCount{0};
        for (const auto &node : d.fixedLayout)
            if (node == "rest")
                ++restCount;
        CHECK(restCount <= 1);
    }
}