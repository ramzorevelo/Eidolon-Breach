/**
 * @file PlayableCharacter.cpp
 * @brief PlayableCharacter implementation.
 */

#include "Entities/PlayableCharacter.h"
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Entities/Party.h"
#include "Items/Item.h"
#include <variant>
#include "UI/IInputHandler.h"
#include "Core/CombatConstants.h"
#include "UI/IRenderer.h"
#include <algorithm>
#include <utility>

PlayableCharacter::PlayableCharacter(std::string id,
                                     std::string name,
                                     Stats stats,
                                     Affinity affinity,
                                     int resonanceContribution,
                                     std::string passiveTrait)
    : Unit{std::move(id), std::move(name), stats, affinity},
      m_resonanceContribution{resonanceContribution},
      m_passiveTrait{std::move(passiveTrait)}
{
}

PlayableCharacter::~PlayableCharacter() = default;

void PlayableCharacter::addAbility(std::unique_ptr<IAction> action)
{
    m_abilities.push_back(std::move(action));
}

const std::vector<std::unique_ptr<IAction>> &PlayableCharacter::getAbilities() const
{
    return m_abilities;
}

void PlayableCharacter::gainEnergy(int amount)
{
    m_resources.energy = std::min(kMaxEnergy, m_resources.energy + amount);
}

void PlayableCharacter::resetEnergy()
{
    m_resources.energy = 0;
}

void PlayableCharacter::consumeEnergy(int amount)
{
    m_resources.energy = std::max(0, m_resources.energy - amount);
}

bool PlayableCharacter::canAffordSp(int amount, const Party &party) const
{
    return party.getSp() >= amount;
}

void PlayableCharacter::consumeSp(int amount, Party &party) const
{
    party.useSp(amount);
}

void PlayableCharacter::modifyExposure(int delta)
{
    m_exposure = std::clamp(m_exposure + delta, 0, kMaxExposure);
}

bool PlayableCharacter::canVent() const
{
    return m_exposure > 0 && m_exposure < kMaxExposure && !hasFlag(CharFlag::Fractured);
}

std::size_t PlayableCharacter::selectActionIndex(const Party &allies,
                                                 IInputHandler &input)
{
    // Build the filtered list of visible ability indices — mirrors renderActionMenu().
    std::vector<std::size_t> visibleIndices{};
    visibleIndices.reserve(m_abilities.size());
    for (std::size_t i{0}; i < m_abilities.size(); ++i)
    {
        const ActionData &data{m_abilities[i]->getActionData()};
        if (data.category == ActionCategory::ArchSkill && !hasFlag(CharFlag::ArchSkillUnlocked))
            continue;
        visibleIndices.push_back(i);
    }

    while (true)
    {
        // Input handler receives the count of visible options, not the raw ability count.
        const std::size_t choice{input.getActionChoice(visibleIndices.size())};
        const std::size_t internalIdx{visibleIndices[choice]};
        if (m_abilities[internalIdx]->isAvailable(*this, allies))
            return internalIdx;
    }
}
std::optional<TargetInfo> PlayableCharacter::selectTarget(const Party &enemies,
                                                          IInputHandler &input,
                                                          IRenderer &renderer)
{
    std::vector<std::pair<std::size_t, const Unit *>> targets{};
    for (std::size_t i{0}; i < enemies.size(); ++i)
    {
        const Unit *u{enemies.getUnitAt(i)};
        if (u && u->isAlive())
            targets.emplace_back(i, u);
    }

    if (targets.empty())
        return std::nullopt;

    std::vector<std::string> names{};
    names.reserve(targets.size());
    for (const auto &[idx, unit] : targets)
        names.push_back(unit->getName());
    renderer.renderTargetList(names, false);
    renderer.renderHintBar("[Up/Down] Cycle  [Enter] Confirm  [Esc] Cancel  / click card");

    const std::size_t choice{input.getTargetChoice(targets.size())};
    if (choice == IInputHandler::kCancelChoice)
        return std::nullopt;
    return TargetInfo{TargetInfo::Type::Enemy, targets[choice].first};
}

std::optional<TargetInfo> PlayableCharacter::selectAllyTarget(const Party &allies,
                                                              IInputHandler &input,
                                                              IRenderer &renderer)
{
    std::vector<std::pair<std::size_t, const Unit *>> targets{};
    for (std::size_t i{0}; i < allies.size(); ++i)
    {
        const Unit *u{allies.getUnitAt(i)};
        if (u && u->isAlive())
            targets.emplace_back(i, u);
    }

    if (targets.empty())
        return std::nullopt;

    std::vector<std::string> names{};
    names.reserve(targets.size());
    for (const auto &[idx, unit] : targets)
        names.push_back(unit->getName());
    renderer.renderTargetList(names, true);
    renderer.renderHintBar("[Up/Down] Cycle  [Enter] Confirm  [Esc] Cancel  / click card");

    const std::size_t choice{input.getTargetChoice(targets.size())};
    if (choice == IInputHandler::kCancelChoice)
        return std::nullopt;
    return TargetInfo{TargetInfo::Type::Ally, targets[choice].first};
}

void PlayableCharacter::displayActionMenu(const Party &party, IRenderer &renderer) const
{
    renderer.renderActionMenu(*this, party);
}

ActionResult PlayableCharacter::takeTurn(Party &allies,
                                         Party &enemies,
                                         BattleState &state)
{
    while (true)
    {
        displayActionMenu(allies, state.renderer);
        const std::size_t actionIdx{selectActionIndex(allies, state.inputHandler)};

        const TargetMode mode{m_abilities[actionIdx]->getActionData().targetMode};
        std::optional<TargetInfo> target{};

        switch (mode)
        {
        case TargetMode::Self:
        case TargetMode::AllEnemies:
        case TargetMode::AllAllies:
            break;
        case TargetMode::SingleAlly:
        case TargetMode::SplashAlly:
            target = selectAllyTarget(allies, state.inputHandler, state.renderer);
            if (!target.has_value())
                continue; // cancelled. Re-show action menu
            break;
        default:
            target = selectTarget(enemies, state.inputHandler, state.renderer);
            if (!target.has_value())
                continue; // cancelled. Re-show action menu
            break;
        }

        const Affinity actionAffinity{m_abilities[actionIdx]->getAffinity()};
        ActionResult result{
            m_abilities[actionIdx]->execute(*this, allies, enemies, target)};
        result.actionAffinity = actionAffinity;
        ++state.turnNumber;
        return result;
    }
}

void PlayableCharacter::tryUnlockSlot(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= EquippedSkillSet::kEquipSlots)
        return;
    m_equipped.slots[static_cast<std::size_t>(slotIndex)].unlocked = true;
}

void PlayableCharacter::equipSkillToSlot(int slotIndex, IAction *skill)
{
    if (slotIndex < 0 || slotIndex >= EquippedSkillSet::kEquipSlots)
        return;
    m_equipped.slots[static_cast<std::size_t>(slotIndex)].equippedSkill = skill;
}

void PlayableCharacter::consumeArchSkill()
{
    m_archSkillCooldown = CombatConstants::kArchSkillCooldownTurns;
}

void PlayableCharacter::tickArchSkillCooldown()
{
    m_archSkillCooldown = std::max(0, m_archSkillCooldown - 1);
}

bool PlayableCharacter::canUseConsumable() const
{
    return m_consumableCooldown == 0 && !hasFlag(CharFlag::ConsumableUsedThisBattle);
}

void PlayableCharacter::consumeConsumableAction(bool multiTurnEffect)
{
    m_consumableCooldown = CombatConstants::kConsumableCooldownTurns;
    if (multiTurnEffect)
        setFlag(CharFlag::ConsumableUsedThisBattle);
}

void PlayableCharacter::tickConsumableCooldown()
{
    m_consumableCooldown = std::max(0, m_consumableCooldown - 1);
}

void PlayableCharacter::resetBattleConsumableState()
{
    m_consumableCooldown = 0;
    m_flags &= ~kBattleResetFlagsMask;
}

void PlayableCharacter::resetArchSkillCooldown()
{
    m_archSkillCooldown = 0;
}

void PlayableCharacter::applyUnlocks(int level)
{
    if (level >= CombatConstants::kArchSkillUnlockLevel)
        setFlag(CharFlag::ArchSkillUnlocked);
    if (level >= CombatConstants::kSlot2UnlockLevel)
        tryUnlockSlot(1);
}

std::optional<Item> PlayableCharacter::equip(const Item &item)
{
    if (item.type != ItemType::Equipment || !item.equipSlot.has_value())
        return std::nullopt;

    // Apply ResonanceModifier immediately at equip time.
    for (const ItemEffect &effect : item.effects)
    {
        if (const auto *rm = std::get_if<ResonanceModifier>(&effect))
            m_resonanceContribution += rm->amount;
    }

    std::optional<Item> displaced{};
    switch (*item.equipSlot)
    {
    case EquipSlot::Weapon:
        displaced = std::move(m_equipment.weapon);
        m_equipment.weapon = item;
        break;
    case EquipSlot::Armor:
        displaced = std::move(m_equipment.armor);
        m_equipment.armor = item;
        break;
    case EquipSlot::Accessory:
        displaced = std::move(m_equipment.accessory);
        m_equipment.accessory = item;
        break;
    }

    // Reverse displaced item's ResonanceModifier.
    if (displaced.has_value())
    {
        for (const ItemEffect &effect : displaced->effects)
        {
            if (const auto *rm = std::get_if<ResonanceModifier>(&effect))
                m_resonanceContribution -= rm->amount;
        }
    }
    return displaced;
}

std::optional<Item> PlayableCharacter::unequip(EquipSlot slot)
{
    std::optional<Item> removed{};
    switch (slot)
    {
    case EquipSlot::Weapon:
        removed = std::move(m_equipment.weapon);
        break;
    case EquipSlot::Armor:
        removed = std::move(m_equipment.armor);
        break;
    case EquipSlot::Accessory:
        removed = std::move(m_equipment.accessory);
        break;
    }

    if (removed.has_value())
    {
        for (const ItemEffect &effect : removed->effects)
        {
            if (const auto *rm = std::get_if<ResonanceModifier>(&effect))
                m_resonanceContribution -= rm->amount;
        }
    }
    return removed;
}

Stats PlayableCharacter::getFinalStats() const
{
    // Pre-pass: flat StatModifier bonuses from equipped items.
    Stats result{getBaseStats()};
    auto applyStatMod = [&result](const Item &item)
    {
        for (const ItemEffect &effect : item.effects)
        {
            if (const auto *sm = std::get_if<StatModifier>(&effect))
            {
                switch (sm->stat)
                {
                case StatModifier::StatType::ATK:
                    result.atk += sm->amount;
                    break;
                case StatModifier::StatType::DEF:
                    result.def += sm->amount;
                    break;
                case StatModifier::StatType::HP:
                    result.hp += sm->amount;
                    result.maxHp += sm->amount;
                    break;
                case StatModifier::StatType::SPD:
                    result.spd += sm->amount;
                    break;
                }
            }
        }
    };

    if (m_equipment.weapon.has_value())
        applyStatMod(*m_equipment.weapon);
    if (m_equipment.armor.has_value())
        applyStatMod(*m_equipment.armor);
    if (m_equipment.accessory.has_value())
        applyStatMod(*m_equipment.accessory);

    // Pass 1: flat additions from IStatusEffect.
    for (const auto &effect : m_effects)
        result = effect->modifyStatsFlat(result);
    // Pass 2: percentage multipliers from IStatusEffect.
    for (const auto &effect : m_effects)
        result = effect->modifyStatsPct(result);

    return result;
}

int PlayableCharacter::getAffinityResistance(Affinity affinity) const
{
    int total{0};
    auto check = [&total, affinity](const std::optional<Item> &slot)
    {
        if (!slot.has_value())
            return;
        for (const ItemEffect &effect : slot->effects)
        {
            if (const auto *ar = std::get_if<AffinityResistance>(&effect))
            {
                if (ar->affinity == affinity)
                    total += ar->flatReduction;
            }
        }
    };
    check(m_equipment.weapon);
    check(m_equipment.armor);
    check(m_equipment.accessory);
    return total;
}