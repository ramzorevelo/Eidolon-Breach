/**
 * @file CharacterRegistry.cpp
 * @brief CharacterRegistry implementation.
 */

#include "Characters/CharacterRegistry.h"
#include "Characters/AbilityRegistry.h"
#include "Core/Affinity.h"
#include "Core/DataLoader.h"
#include "Actions/VentAction.h"
#include "Entities/PlayableCharacter.h"
#include <stdexcept>

void CharacterRegistry::loadFromJson(const std::string &jsonPath,
                                     const AbilityRegistry &abilityRegistry)
{
    m_abilityRegistry = &abilityRegistry;
    const nlohmann::json j{DataLoader::loadJson(jsonPath)};
    for (const auto &entry : j) 
    {
        const std::string id{entry.at("id").get<std::string>()};
        m_blueprints[id] = parseBlueprint(id, entry);
        m_orderedIds.push_back(id);
    }
}

std::unique_ptr<PlayableCharacter>
CharacterRegistry::create(std::string_view characterId, int characterLevel) const
{
    auto it = m_blueprints.find(std::string{characterId});
    if (it == m_blueprints.end())
        return nullptr;

    const CharacterBlueprint &bp{it->second};
    Stats stats{bp.maxHp, bp.maxHp, bp.atk, bp.def, bp.spd};
    auto pc{std::make_unique<PlayableCharacter>(
        bp.id, bp.name, stats, parseAffinity(bp.affinity),
        bp.resonanceContribution, bp.passiveTrait)};

    pc->setArchetype(bp.archetype);

    // Resolve abilities; log warning for unregistered IDs but do not throw,
    // so partial configurations can still be tested.
    if (m_abilityRegistry)
    {
        if (auto basic{m_abilityRegistry->create(bp.basicId)})
            pc->addAbility(std::move(basic));
        if (auto arch{m_abilityRegistry->create(bp.archSkillId)})
            pc->addAbility(std::move(arch));
        if (auto ult{m_abilityRegistry->create(bp.ultimateId)})
            pc->addAbility(std::move(ult));
    }

    for (std::size_t i{0}; i < bp.slotSkillIds.size(); ++i)
    {
        if (i >= static_cast<std::size_t>(EquippedSkillSet::kEquipSlots))
            break;
        if (!m_abilityRegistry)
            break;
        auto skill{m_abilityRegistry->create(bp.slotSkillIds[i])};
        if (!skill)
            continue;
        IAction *rawPtr{skill.get()};
        pc->addAbility(std::move(skill));
        pc->tryUnlockSlot(static_cast<int>(i));
        pc->equipSkillToSlot(static_cast<int>(i), rawPtr);
    }

    pc->addAbility(std::make_unique<VentAction>());

    pc->applyUnlocks(characterLevel);

    return pc;
}

const std::vector<std::string> &CharacterRegistry::getIds() const
{
    return m_orderedIds;
}

bool CharacterRegistry::contains(std::string_view characterId) const
{
    return m_blueprints.count(std::string{characterId}) > 0;
}

std::string CharacterRegistry::getArchetype(std::string_view characterId) const
{
    auto it{m_blueprints.find(std::string{characterId})};
    return (it != m_blueprints.end()) ? it->second.archetype : "";
}

CharacterRegistry::CharacterBlueprint
CharacterRegistry::parseBlueprint(const std::string &id, const nlohmann::json &j)
{
    CharacterBlueprint bp{};
    bp.id = id;
    bp.name = j.at("name").get<std::string>();
    bp.affinity = j.at("affinity").get<std::string>();
    bp.maxHp = j.at("maxHp").get<int>();
    bp.atk = j.at("atk").get<int>();
    bp.def = j.at("def").get<int>();
    bp.spd = j.at("spd").get<int>();
    bp.resonanceContribution = j.value("resonanceContribution", 10);
    bp.passiveTrait = j.value("passiveTrait", "");
    bp.archetype = j.value("archetype", "");

    const auto &abilities = j.at("abilities");
    bp.basicId = abilities.value("basic", "basic_strike");
    bp.archSkillId = abilities.value("archSkill", "arch_skill_default");
    bp.ultimateId = abilities.value("ultimate", "ultimate_default");
    for (const auto &skillId : abilities.value("slotSkills", nlohmann::json::array()))
        bp.slotSkillIds.push_back(skillId.get<std::string>());
    return bp;
}

Affinity CharacterRegistry::parseAffinity(const std::string &s)
{
    if (s == "Blaze")
        return Affinity::Blaze;
    if (s == "Frost")
        return Affinity::Frost;
    if (s == "Tempest")
        return Affinity::Tempest;
    if (s == "Terra")
        return Affinity::Terra;
    return Affinity::Aether;
}