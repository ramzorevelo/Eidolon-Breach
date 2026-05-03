/**
 * @file test_MetaProgress.cpp
 * @brief Tests for MetaProgress: save/load round-trip, unlockCharacter echo cap,
 *        gainXP level-up, levelFromXP formula (non‑linear), player level system.
 */
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "doctest.h"
#include <filesystem>

// ── levelFromXP (non‑linear) ─────────────────────────────────────────────────

TEST_CASE("MetaProgress::levelFromXP: level 1 at 0 XP")
{
    CHECK(MetaProgress::levelFromXP(0) == 1);
}

TEST_CASE("MetaProgress::levelFromXP: level 2 after 50 XP")
{
    // xpToLevel(1) = floor(50 * 1^1.5) = 50
    CHECK(MetaProgress::levelFromXP(50) == 2);
}

TEST_CASE("MetaProgress::levelFromXP: level 3 requires cumulative 191 XP")
{
    // xpToLevel(1) = 50
    // xpToLevel(2) = floor(50 * 2^1.5) = floor(141.4) = 141
    // Cumulative: 50 + 141 = 191
    CHECK(MetaProgress::levelFromXP(190) == 2);
    CHECK(MetaProgress::levelFromXP(191) == 3);
}

TEST_CASE("MetaProgress::levelFromXP: 200 XP is still level 3 (not enough for level 4)")
{
    // xpToLevel(3) = floor(50 * 3^1.5) = floor(259.8) = 259
    // Cumulative: 50 + 141 + 259 = 450 for level 4
    CHECK(MetaProgress::levelFromXP(200) == 3);
}

// ── playerLevelFromXp ────────────────────────────────────────────────────────

TEST_CASE("MetaProgress::playerLevelFromXp: level 1 at 0 XP")
{
    CHECK(MetaProgress::playerLevelFromXp(0) == 1);
}

TEST_CASE("MetaProgress::playerLevelFromXp: level 2 after 100 XP")
{
    // xpToLevel(1) = floor(100 * 1^1.8) = 100
    CHECK(MetaProgress::playerLevelFromXp(100) == 2);
}

TEST_CASE("MetaProgress::gainPlayerXp: updates playerLevel correctly")
{
    MetaProgress meta{};
    CHECK(meta.playerLevel == 1);
    meta.gainPlayerXp(100);
    CHECK(meta.playerLevel == 2);
}

// ── gainXP (character XP, non‑linear) ────────────────────────────────────────

TEST_CASE("MetaProgress::gainXP: returns new level after 50 XP")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");

    const int newLevel{meta.gainXP("hero", 50)};
    CHECK(newLevel == 2);
    CHECK(meta.characterXP["hero"] == 50);
    CHECK(meta.characterLevels["hero"] == 2);
}

TEST_CASE("MetaProgress::gainXP: accumulates across multiple calls")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");

    meta.gainXP("hero", 30);
    meta.gainXP("hero", 30);
    CHECK(meta.characterXP["hero"] == 60);
    CHECK(meta.characterLevels["hero"] == 2); // 60 > 50, level 2
}

TEST_CASE("MetaProgress::gainXP: level stays at 1 below 50 XP")
{
    MetaProgress meta{};
    meta.unlockCharacter("hero");

    meta.gainXP("hero", 49);
    CHECK(meta.characterLevels["hero"] == 1);
}

// ── unlockCharacter ──────────────────────────────────────────────────────────

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
    meta.unlockCharacter("hero");
    for (int i{0}; i < CombatConstants::kMaxEchoes + 5; ++i)
        meta.unlockCharacter("hero");
    CHECK(meta.characterInsight["hero"].echoCount == CombatConstants::kMaxEchoes);
}

// ── save / load round‑trip ───────────────────────────────────────────────────

TEST_CASE("MetaProgress: save/load round-trip preserves all fields")
{
    const std::filesystem::path testPath{"test_save_roundtrip.json"};

    MetaProgress original{};
    original.currency = 150;
    original.highestFloorReached = 7;
    original.draftModeUnlocked = true;
    original.unlockCharacter("lyra");
    original.gainXP("lyra", 200);
    original.masteryEventLog["lyra"].push_back("lyra_predator");
    original.characterInsight["lyra"].echoCount = 2;

    original.saveToFile(testPath);

    const MetaProgress loaded{MetaProgress::loadFromFile(testPath)};
    CHECK(loaded.currency == 150);
    CHECK(loaded.highestFloorReached == 7);
    CHECK(loaded.draftModeUnlocked == true);
    CHECK(loaded.unlockedCharacterIds.count("lyra") == 1);
    CHECK(loaded.characterXP.at("lyra") == 200);
    // 200 XP = level 3 (thresholds: 50 + 141 = 191 for level 3)
    CHECK(loaded.characterLevels.at("lyra") == 3);
    CHECK(loaded.masteryEventLog.at("lyra")[0] == "lyra_predator");
    CHECK(loaded.characterInsight.at("lyra").echoCount == 2);

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
    // 150 XP = level 2 (50 for level 2, remaining 100 < 141 for level 3)
    CHECK(reloaded.characterLevels.at("hero") == 2);

    std::filesystem::remove(testPath);
}