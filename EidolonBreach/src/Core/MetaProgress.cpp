/**
 * @file MetaProgress.cpp
 * @brief MetaProgress implementation — save/load, unlock, XP, and level-up.
 */

#include "Core/MetaProgress.h"
#include "Core/RunContext.h"
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include "Meta/AspectTree.h"
#include <algorithm>
#include <fstream>
#include <cmath>

namespace
{
BehaviorSignal behaviorSignalFromString(const std::string &s)
{
    if (s == "Aggressive")
        return BehaviorSignal::Aggressive;
    if (s == "Methodical")
        return BehaviorSignal::Methodical;
    if (s == "Sacrificial")
        return BehaviorSignal::Sacrificial;
    if (s == "Supportive")
        return BehaviorSignal::Supportive;
    return BehaviorSignal::Reactive;
}

std::string behaviorSignalToString(BehaviorSignal s)
{
    switch (s)
    {
    case BehaviorSignal::Aggressive:
        return "Aggressive";
    case BehaviorSignal::Methodical:
        return "Methodical";
    case BehaviorSignal::Sacrificial:
        return "Sacrificial";
    case BehaviorSignal::Supportive:
        return "Supportive";
    case BehaviorSignal::Reactive:
        return "Reactive";
    }
    return "Reactive";
}
} // namespace

int MetaProgress::levelFromXP(int totalXP)
{
    if (totalXP <= 0)
        return 1;
    int level{1};
    int cumulative{0};
    while (true)
    {
        const int threshold{static_cast<int>(
            CombatConstants::kXpLevelBase *
            std::pow(static_cast<float>(level), CombatConstants::kXpLevelExponent))};
        if (cumulative + threshold > totalXP)
            return level;
        cumulative += threshold;
        ++level;
    }
}

int MetaProgress::playerLevelFromXp(int totalXp)
{
    if (totalXp <= 0)
        return 1;
    int level{1};
    int cumulative{0};
    while (true)
    {
        const int threshold{static_cast<int>(
            CombatConstants::kPlayerXpLevelBase *
            std::pow(static_cast<float>(level), CombatConstants::kPlayerXpLevelExponent))};
        if (cumulative + threshold > totalXp)
            return level;
        cumulative += threshold;
        ++level;
    }
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

    // Store in named variables before calling .items() — j.value() returns a
    // temporary whose lifetime does not extend through the range-for loop.
    const nlohmann::json characterXP{j.value("characterXP", nlohmann::json::object())};
    for (const auto &[id, xp] : characterXP.items())
    {
        meta.characterXP[id] = xp.get<int>();
        meta.characterLevels[id] = levelFromXP(xp.get<int>());
    }

    meta.playerXp = j.value("playerXp", 0);
    meta.playerLevel = j.value("playerLevel", 1);
    for (const auto &id : j.value("clearedDungeonIds", nlohmann::json::array()))
        meta.clearedDungeonIds.insert(id.get<std::string>());

    const nlohmann::json insightJson{j.value("characterInsight", nlohmann::json::object())};
    for (const auto &[id, insight] : insightJson.items())
    {
        CharacterInsightData data{};
        data.echoCount = insight.value("echoCount", 0);
        data.bondTrialComplete = insight.value("bondTrialComplete", false);
        const nlohmann::json aspects{insight.value("chosenAspects", nlohmann::json::array())};
        for (const auto &aspect : aspects)
            data.chosenAspects.push_back(aspect.get<std::string>());
        data.insightBalance = insight.value("insightBalance", 0);
        const nlohmann::json tallies{insight.value("signalTallies", nlohmann::json::object())};
        for (const auto &[sigStr, count] : tallies.items())
            data.signalTallies[behaviorSignalFromString(sigStr)] = count.get<int>();
        meta.characterInsight[id] = std::move(data);
    }

    const nlohmann::json masteryJson{j.value("masteryEventLog", nlohmann::json::object())};
    for (const auto &[id, log] : masteryJson.items())
    {
        for (const auto &entry : log)
            meta.masteryEventLog[id].push_back(entry.get<std::string>());
    }

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

    j["playerXp"] = playerXp;
    j["playerLevel"] = playerLevel;
    j["clearedDungeonIds"] = nlohmann::json::array();
    for (const auto &id : clearedDungeonIds)
        j["clearedDungeonIds"].push_back(id);

    j["characterInsight"] = nlohmann::json::object();
    for (const auto &[id, data] : characterInsight)
    {
        j["characterInsight"][id]["echoCount"] = data.echoCount;
        j["characterInsight"][id]["bondTrialComplete"] = data.bondTrialComplete;
        j["characterInsight"][id]["chosenAspects"] = data.chosenAspects;
        j["characterInsight"][id]["insightBalance"] = data.insightBalance;
        nlohmann::json talliesJson{nlohmann::json::object()};
        for (const auto &[signal, count] : data.signalTallies)
            talliesJson[behaviorSignalToString(signal)] = count;
        j["characterInsight"][id]["signalTallies"] = talliesJson;
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

int MetaProgress::gainPlayerXp(int amount)
{
    playerXp += amount;
    playerLevel = playerLevelFromXp(playerXp);
    return playerLevel;
}

void MetaProgress::gainRunSignals(const RunContext &ctx, const Party &party)
{
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        const Unit *u{party.getUnitAt(i)};
        if (!u)
            continue;
        const RunCharacterState *cs{ctx.findCharacterState(u->getId())};
        if (!cs)
            continue;

        auto &insight{characterInsight[std::string{u->getId()}]};
        int totalSignals{0};
        for (const auto &[signal, count] : cs->signalCounts)
        {
            insight.signalTallies[signal] += count;
            totalSignals += count;
        }
        insight.insightBalance += totalSignals / CombatConstants::kInsightPerSignalPoints;
    }
}

bool MetaProgress::spendInsight(std::string_view characterId,
                                std::string_view nodeId,
                                const AspectTree &tree)
{
    const AspectTreeNode *node{tree.findNode(nodeId)};
    if (!node)
        return false;

    auto &insight{characterInsight[std::string{characterId}]};

    const auto tallyIt{insight.signalTallies.find(node->branch)};
    const int tally{(tallyIt != insight.signalTallies.end()) ? tallyIt->second : 0};
    if (tally < node->unlockThreshold)
        return false;

    for (const auto &chosen : insight.chosenAspects)
        if (chosen == node->id)
            return false;

    if (insight.insightBalance < node->insightCost)
        return false;

    insight.insightBalance -= node->insightCost;
    insight.chosenAspects.push_back(std::string{node->id});
    return true;
}