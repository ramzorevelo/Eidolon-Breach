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
#include "Core/MetaProgress.h"
#include "Meta/AspectTree.h"
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
CharacterRegistry::create(std::string_view characterId,
                          int characterLevel,
                          const MetaProgress *meta) const
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
    pc->setFractureSelfDotPct(bp.fractureSelfDotPct);
    pc->setBreachbornActionBonus(bp.breachbornActionBonusDivisor,
                                 bp.breachbornActionBurnDamage,
                                 bp.breachbornActionBurnDuration);

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

    // Apply Aspect Tree node effects and Echo effects before unlock resolution,
    // so SlotUnlockEarly mods take effect when applyUnlocks is called below.
    if (meta)
    {
        applyProgressionMods(characterId, *meta, *pc);
        applyEchoEffects(characterId,
                         meta->characterInsight.count(std::string{characterId})
                             ? meta->characterInsight.at(std::string{characterId}).echoCount
                             : 0,
                         *pc);
    }

    pc->applyUnlocks(characterLevel);

    return pc;
}

void CharacterRegistry::applyProgressionMods(std::string_view characterId,
                                             const MetaProgress &meta,
                                             PlayableCharacter &pc)
{
    const auto it{meta.characterInsight.find(std::string{characterId})};
    if (it == meta.characterInsight.end())
        return;

    const AspectTree tree{AspectTree::loadForCharacter(characterId)};
    for (const auto &nodeId : it->second.chosenAspects)
    {
        const AspectTreeNode *node{tree.findNode(nodeId)};
        if (node)
            pc.applyCharacterMod(node->effect);
    }
}

void CharacterRegistry::applyEchoEffects(std::string_view characterId,
                                         int echoCount,
                                         PlayableCharacter &pc)
{
    if (echoCount <= 0)
        return;

    // Echo 1: Basic attack strengthened: small ATK bonus.
    if (echoCount >= 1)
        pc.applyCharacterMod(StatBonus{0, 2, 0, 0});

    // Echo 2: Slot skill enhanced: proc effects are stronger.
    if (echoCount >= 2)
        pc.applyCharacterMod(ProcEnhancement{1.10f});

    // Echo 3: Always-active passive: stat bonus independent of stance/aspect.
    if (echoCount >= 3)
    {
        if (characterId == "lyra")
            pc.applyCharacterMod(StatBonus{0, 2, 0, 1});
        else if (characterId == "vex")
            pc.applyCharacterMod(StatBonus{5, 0, 2, 0});
        else if (characterId == "zara")
            pc.applyCharacterMod(StatBonus{0, 1, 1, 2});
        else
            pc.applyCharacterMod(StatBonus{3, 1, 1, 0});
    }

    // Echo 4: Fracture state modified: reduced AV penalty.
    if (echoCount >= 4)
        pc.applyCharacterMod(AVModifierBonus{ExposureState::Fractured, -0.05f});

    // Echo 5: Ultimate augmented: set flag so Battle applies enhanced output.
    if (echoCount >= 5)
        pc.applyCharacterMod(ProcEnhancement{1.15f});
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
    bp.fractureSelfDotPct = j.value("fractureSelfDotPct", 0.0f);
    bp.breachbornActionBonusDivisor = j.value("breachbornActionBonusDivisor", 0.0f);
    bp.breachbornActionBurnDamage = j.value("breachbornActionBurnDamage", 0);
    bp.breachbornActionBurnDuration = j.value("breachbornActionBurnDuration", 0);

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