/**
 * @file PlayableCharacter.cpp
 * @brief PlayableCharacter implementation.
 */

#include "Entities/PlayableCharacter.h"
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Entities/Party.h"
#include "UI/IInputHandler.h"
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

bool PlayableCharacter::isArchSkillReady() const
{
    // Placeholder; will be cooldown-gated in future hotfix
    return true;
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
    return m_exposure > 0 && m_exposure < kMaxExposure;
}

std::size_t PlayableCharacter::selectActionIndex(const Party &allies,
                                                 IInputHandler &input)
{
    while (true)
    {
        std::size_t idx{input.getActionChoice(m_abilities.size())};
        if (m_abilities[idx]->isAvailable(*this, allies))
            return idx;
    }
}

std::optional<TargetInfo> PlayableCharacter::selectTarget(const Party &enemies,
                                                          IInputHandler &input)
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
    if (targets.size() == 1)
        return TargetInfo{TargetInfo::Type::Enemy, targets[0].first};

    std::size_t choice{input.getTargetChoice(targets.size())};
    return TargetInfo{TargetInfo::Type::Enemy, targets[choice].first};
}

void PlayableCharacter::displayActionMenu(const Party &party, IRenderer &renderer) const
{
    renderer.renderActionMenu(*this, party);
}

ActionResult PlayableCharacter::takeTurn(Party &allies,
                                         Party &enemies,
                                         BattleState &state)
{
    displayActionMenu(allies, state.renderer);
    std::size_t actionIdx{selectActionIndex(allies, state.inputHandler)};
    std::optional<TargetInfo> target{selectTarget(enemies, state.inputHandler)};

    ActionResult result{m_abilities[actionIdx]->execute(*this, allies, enemies, target)};

    state.resonanceField.addContribution(
        m_abilities[actionIdx]->getAffinity(),
        m_resonanceContribution);
    ++state.turnNumber;

    return result;
}