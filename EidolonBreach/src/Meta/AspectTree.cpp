/**
 * @file AspectTree.cpp
 * @brief AspectTree implementation — JSON loading and node lookup.
 */
#include "Meta/AspectTree.h"
#include "Core/DataLoader.h"
#include "nlohmann/json.hpp"
#include <stdexcept>
#include <string>

namespace
{
BehaviorSignal signalFromString(const std::string &s)
{
    if (s == "Aggressive")
        return BehaviorSignal::Aggressive;
    if (s == "Methodical")
        return BehaviorSignal::Methodical;
    if (s == "Sacrificial")
        return BehaviorSignal::Sacrificial;
    if (s == "Supportive")
        return BehaviorSignal::Supportive;
    if (s == "Reactive")
        return BehaviorSignal::Reactive;
    throw std::runtime_error{"Unknown BehaviorSignal: " + s};
}

ExposureState exposureStateFromString(const std::string &s)
{
    if (s == "Baseline")
        return ExposureState::Baseline;
    if (s == "Resonating")
        return ExposureState::Resonating;
    if (s == "Surging")
        return ExposureState::Surging;
    if (s == "Breachborn")
        return ExposureState::Breachborn;
    if (s == "Fractured")
        return ExposureState::Fractured;
    throw std::runtime_error{"Unknown ExposureState: " + s};
}

CharacterMod parseEffect(const nlohmann::json &e)
{
    const std::string type{e.at("type").get<std::string>()};
    if (type == "StatBonus")
    {
        StatBonus b{};
        b.hp = e.value("hp", 0);
        b.atk = e.value("atk", 0);
        b.def = e.value("def", 0);
        b.spd = e.value("spd", 0);
        return b;
    }
    if (type == "CooldownReduction")
        return CooldownReduction{e.at("turns").get<int>()};
    if (type == "ExposureThresholdShift")
        return ExposureThresholdShift{e.at("threshold").get<int>(),
                                      e.at("delta").get<int>()};
    if (type == "SlotUnlockEarly")
        return SlotUnlockEarly{e.at("slot").get<int>(),
                               e.at("levelRequired").get<int>()};
    if (type == "ProcEnhancement")
        return ProcEnhancement{e.at("multiplier").get<float>()};
    if (type == "AVModifierBonus")
        return AVModifierBonus{exposureStateFromString(e.at("state").get<std::string>()),
                               e.at("delta").get<float>()};
    throw std::runtime_error{"Unknown CharacterMod type: " + type};
}

AspectTreeNode parseNode(const nlohmann::json &j)
{
    AspectTreeNode node{};
    node.id = j.at("id").get<std::string>();
    node.branch = signalFromString(j.at("branch").get<std::string>());
    node.unlockThreshold = j.at("unlockThreshold").get<int>();
    node.insightCost = j.at("insightCost").get<int>();
    node.effect = parseEffect(j.at("effect"));
    return node;
}
} // namespace

AspectTree AspectTree::loadForCharacter(std::string_view characterId)
{
    const std::string path{"data/aspect_trees/" + std::string{characterId} + ".json"};
    AspectTree tree{};
    try
    {
        const nlohmann::json j{DataLoader::loadJson(path)};
        for (const auto &entry : j)
            tree.m_nodes.push_back(parseNode(entry));
    }
    catch (...)
    {
        // Missing or malformed file — return an empty tree; caller handles it.
    }
    return tree;
}

const std::vector<AspectTreeNode> &AspectTree::nodes() const
{
    return m_nodes;
}

const AspectTreeNode *AspectTree::findNode(std::string_view nodeId) const
{
    for (const auto &n : m_nodes)
        if (n.id == nodeId)
            return &n;
    return nullptr;
}