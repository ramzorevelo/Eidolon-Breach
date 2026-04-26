/**
 * @file MetaProgress.cpp
 * @brief MetaProgress implementation — save/load, unlock, XP, and level-up.
 */

#include "Core/MetaProgress.h"
#include <algorithm>
#include <fstream>


int MetaProgress::levelFromXP(int totalXP)
{
    return 1 + totalXP / CombatConstants::kXpPerLevel;
}


MetaProgress MetaProgress::loadFromFile(const std::filesystem::path &path)
{
    MetaProgress meta{};
    std::ifstream file{path};
    if (!file.is_open())
        return meta; 

    nlohmann::json j{};
    try
    {
        file >> j;
    }
    catch (const nlohmann::json::parse_error &)
    {
        return meta; 
    }

    meta.currency = j.value("currency", 0);
    meta.highestFloorReached = j.value("highestFloorReached", 0);
    meta.draftModeUnlocked = j.value("draftModeUnlocked", false);

    for (const auto &id : j.value("unlockedCharacterIds", nlohmann::json::array()))
        meta.unlockedCharacterIds.insert(id.get<std::string>());

    for (const auto &[id, xp] : j.value("characterXP", nlohmann::json::object()).items())
    {
        meta.characterXP[id] = xp.get<int>();
        meta.characterLevels[id] = levelFromXP(xp.get<int>());
    }

    for (const auto &[id, insight] : j.value("characterInsight", nlohmann::json::object()).items())
    {
        CharacterInsightData data{};
        data.echoCount = insight.value("echoCount", 0);
        data.bondTrialComplete = insight.value("bondTrialComplete", false);
        for (const auto &aspect : insight.value("chosenAspects", nlohmann::json::array()))
            data.chosenAspects.push_back(aspect.get<std::string>());
        meta.characterInsight[id] = std::move(data);
    }

    for (const auto &[id, log] : j.value("masteryEventLog", nlohmann::json::object()).items())
        for (const auto &entry : log)
            meta.masteryEventLog[id].push_back(entry.get<std::string>());

    return meta;
}

void MetaProgress::saveToFile(const std::filesystem::path &path) const
{
    nlohmann::json j{};
    j["currency"] = currency;
    j["highestFloorReached"] = highestFloorReached;
    j["draftModeUnlocked"] = draftModeUnlocked;

    j["unlockedCharacterIds"] = nlohmann::json::array();
    for (const auto &id : unlockedCharacterIds)
        j["unlockedCharacterIds"].push_back(id);

    j["characterXP"] = nlohmann::json::object();
    for (const auto &[id, xp] : characterXP)
        j["characterXP"][id] = xp;

    j["characterInsight"] = nlohmann::json::object();
    for (const auto &[id, data] : characterInsight)
    {
        j["characterInsight"][id]["echoCount"] = data.echoCount;
        j["characterInsight"][id]["bondTrialComplete"] = data.bondTrialComplete;
        j["characterInsight"][id]["chosenAspects"] = data.chosenAspects;
    }

    j["masteryEventLog"] = nlohmann::json::object();
    for (const auto &[id, log] : masteryEventLog)
        j["masteryEventLog"][id] = log;

    std::ofstream file{path};
    if (file.is_open())
        file << j.dump(4);
}


bool MetaProgress::unlockCharacter(std::string_view characterId)
{
    const std::string id{characterId};
    if (unlockedCharacterIds.count(id) == 0)
    {
        unlockedCharacterIds.insert(id);
        characterLevels[id] = 1;
        characterXP[id] = 0;
        return true;
    }

    // Already unlocked — award an Echo instead (capped at kMaxEchoes).
    auto &insight = characterInsight[id];
    if (insight.echoCount < CombatConstants::kMaxEchoes)
        ++insight.echoCount;

    return false;
}


int MetaProgress::gainXP(std::string_view characterId, int amount)
{
    const std::string id{characterId};
    characterXP[id] += amount;
    characterLevels[id] = levelFromXP(characterXP[id]);
    return characterLevels[id];
}