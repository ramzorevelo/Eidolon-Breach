/**
 * @file test_MetaProgress.cpp
 * @brief Tests for MetaProgress: save/load round-trip, unlockCharacter echo cap,
 *        gainXP level-up, levelFromXP formula.
 */
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "doctest.h"
#include <filesystem>

// ── levelFromXP ───────────────────────────────────────────────────────────────

TEST_CASE("MetaProgress::levelFromXP: level 1 at 0 XP")
{
    CHECK(MetaProgress::levelFromXP(0) == 1);
}

TEST_CASE("MetaProgress::levelFromXP: level 2 after kXpPerLevel XP")
{
    CHECK(MetaProgress::levelFromXP(CombatConstants::kXpPerLevel) == 2);
}

TEST_CASE("MetaProgress::levelFromXP: level 20 threshold")
{
    const int xpNeeded{(CombatConstants::kSlot1UnlockLevel - 1) *
                       CombatConstants::kXpPerLevel};
    CHECK(MetaProgress::levelFromXP(xpNeeded) == CombatConstants::kSlot1UnlockLevel);
}

TEST_CASE("MetaProgress::levelFromXP: level 40 threshold")
{
    const int xpNeeded{(CombatConstants::kSlot2UnlockLevel - 1) *
                       CombatConstants::kXpPerLevel};
    CHECK(MetaProgress::levelFromXP(xpNeeded) == CombatConstants::kSlot2UnlockLevel);
}

// ── gainXP ────────────────────────────────────────────────────────────────────

TEST_CASE("MetaProgress::gainXP: returns new level after XP gain")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");

    const int newLevel{meta.gainXP("hero", CombatConstants::kXpPerLevel)};
    CHECK(newLevel == 2);
    CHECK(meta.characterXP["hero"] == CombatConstants::kXpPerLevel);
    CHECK(meta.characterLevels["hero"] == 2);
}

TEST_CASE("MetaProgress::gainXP: accumulates across multiple calls")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");

    meta.gainXP("hero", 30);
    meta.gainXP("hero", 30);
    CHECK(meta.characterXP["hero"] == 60);
    // 60 / 50 = 1, so level = 2
    CHECK(meta.characterLevels["hero"] == 2);
}

TEST_CASE("MetaProgress::gainXP: level stays at 1 below threshold")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");

    meta.gainXP("hero", CombatConstants::kXpPerLevel - 1);
    CHECK(meta.characterLevels["hero"] == 1);
}

// ── unlockCharacter ───────────────────────────────────────────────────────────

TEST_CASE("MetaProgress::unlockCharacter: returns true for new character")
{
    MetaProgress meta{};
    CHECK(meta.unlockCharacter("hero") == true);
    CHECK(meta.unlockedCharacterIds.count("hero") == 1);
}

TEST_CASE("MetaProgress::unlockCharacter: returns false for already-unlocked character")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");
    CHECK(meta.unlockCharacter("hero") == false);
}

TEST_CASE("MetaProgress::unlockCharacter: awards Echo on duplicate unlock")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");
    meta.unlockCharacter("hero");
    CHECK(meta.characterInsight["hero"].echoCount == 1);
}

TEST_CASE("MetaProgress::unlockCharacter: echo count capped at kMaxEchoes")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero"); // unlock
    for (int i{0}; i < CombatConstants::kMaxEchoes + 5; ++i)
        meta.unlockCharacter("hero"); // try to exceed cap
    CHECK(meta.characterInsight["hero"].echoCount == CombatConstants::kMaxEchoes);
}

// ── save / load round-trip ────────────────────────────────────────────────────

TEST_CASE("MetaProgress: save/load round-trip preserves all fields")
{
    const std::filesystem::path testPath{"test_save_roundtrip.json"};

    MetaProgress original{};
    original.currency = 150;
    original.highestFloorReached = 7;
    original.draftModeUnlocked = true;
    original.unlockCharacter("striker_1");
    original.gainXP("striker_1", 200);
    original.masteryEventLog["striker_1"].push_back("lyra_predator");
    original.characterInsight["striker_1"].echoCount = 2;

    original.saveToFile(testPath);

    const MetaProgress loaded{MetaProgress::loadFromFile(testPath)};
    CHECK(loaded.currency == 150);
    CHECK(loaded.highestFloorReached == 7);
    CHECK(loaded.draftModeUnlocked == true);
    CHECK(loaded.unlockedCharacterIds.count("striker_1") == 1);
    CHECK(loaded.characterXP.at("striker_1") == 200);
    CHECK(loaded.characterLevels.at("striker_1") == 5); // 200/50 + 1
    CHECK(loaded.masteryEventLog.at("striker_1")[0] == "lyra_predator");
    CHECK(loaded.characterInsight.at("striker_1").echoCount == 2);

    std::filesystem::remove(testPath);
}

TEST_CASE("MetaProgress::loadFromFile: returns defaults when file does not exist")
{
    const MetaProgress meta{MetaProgress::loadFromFile("nonexistent_save.json")};
    CHECK(meta.currency == 0);
    CHECK(meta.highestFloorReached == 0);
    CHECK(meta.unlockedCharacterIds.empty());
}

TEST_CASE("MetaProgress: multiple save/load cycles preserve XP correctly")
{
    const std::filesystem::path testPath{"test_save_multi.json"};

    MetaProgress meta{};
    meta.unlockCharacter("hero");
    meta.gainXP("hero", 100);
    meta.saveToFile(testPath);

    MetaProgress loaded{MetaProgress::loadFromFile(testPath)};
    loaded.gainXP("hero", 50);
    loaded.saveToFile(testPath);

    const MetaProgress reloaded{MetaProgress::loadFromFile(testPath)};
    CHECK(reloaded.characterXP.at("hero") == 150);
    CHECK(reloaded.characterLevels.at("hero") == 4); // 150/50 + 1

    std::filesystem::remove(testPath);
}