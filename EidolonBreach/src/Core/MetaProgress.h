#pragma once
/**
 * @file MetaProgress.h
 * @brief Cross-run persistent state. Injected by reference — never a singleton.
 *        loadFromFile is a static named constructor returning a value.
 *        Serialized to save.json via nlohmann/json.
 */

#include "Core/CombatConstants.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

/** Per-character insight data stored in MetaProgress. */
struct CharacterInsightData
{
    int echoCount{0};                         ///< Resonance Echoes earned; capped at kMaxEchoes.
    bool bondTrialComplete{false};            ///< Phase 9.
    std::vector<std::string> chosenAspects{}; ///< Phase 9 Aspect Tree nodes.
};

class MetaProgress
{
  public:
    /**
     * @brief Load persistent state from a JSON file.
     *        Returns a default MetaProgress if the file does not exist.
     * @param path File path (e.g. "save.json").
     */
    [[nodiscard]] static MetaProgress loadFromFile(const std::filesystem::path &path);

    /**
     * @brief Serialize state to a JSON file. Creates the file if absent.
     * @param path File path (e.g. "save.json").
     */
    void saveToFile(const std::filesystem::path &path) const;

    /**
     * @brief Unlock a character by ID.
     * @return true if newly unlocked; false if already unlocked (Echo awarded instead,
     *         capped at kMaxEchoes).
     */
    bool unlockCharacter(std::string_view characterId);

    /**
     * @brief Award XP to a character and apply level-up if threshold crossed.
     * @param characterId Unit::getId() of the character.
     * @param amount      XP to award.
     * @return The character's new level after applying XP.
     */
    int gainXP(std::string_view characterId, int amount);

    /**
     * @brief Compute level from total accumulated XP.
     *        Level = 1 + totalXP / kXpPerLevel (integer division, minimum 1).
     */
    [[nodiscard]] static int levelFromXP(int totalXP);

    int currency{0};
    int highestFloorReached{0};
    bool draftModeUnlocked{false};
    std::set<std::string> unlockedCharacterIds{};
    std::map<std::string, int> characterLevels{};                      ///< id → current level
    std::map<std::string, int> characterXP{};                          ///< id → total XP
    std::map<std::string, CharacterInsightData> characterInsight{};    ///< id → insight data
    std::map<std::string, std::vector<std::string>> masteryEventLog{}; ///< id → stance IDs logged
};