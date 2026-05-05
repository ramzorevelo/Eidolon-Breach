#pragma once
/**
 * @file MetaProgress.h
 * @brief Cross-run persistent state. Injected by reference.
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
    int echoCount{0};                          
    bool bondTrialComplete{false};            
    std::vector<std::string> chosenAspects{}; 
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
    /**
     * @brief Award player (account) XP and update playerLevel.
     * @param amount Raw XP to add before level recomputation.
     * @return New player level.
     */
    int gainPlayerXp(int amount);

    /**
     * @brief Compute account level from total player XP.
     *        Uses a steeper curve than character XP (kPlayerXpLevelExponent = 1.8).
     */
    [[nodiscard]] static int playerLevelFromXp(int totalXp);

    int currency{0};
    int highestFloorReached{0};
    bool draftModeUnlocked{false};
    std::set<std::string> unlockedCharacterIds{};
    std::map<std::string, int> characterLevels{};                     
    std::map<std::string, int> characterXP{};                          
    std::map<std::string, CharacterInsightData> characterInsight{};    
    std::map<std::string, std::vector<std::string>> masteryEventLog{}; 
    int playerXp{0};                                                   
    int playerLevel{1};                                                
    std::set<std::string> clearedDungeonIds{};                         
};