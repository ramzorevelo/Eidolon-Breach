#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <utility>

PlayableCharacter::PlayableCharacter(std::string id,
    std::string name,
    Stats       stats,
    Affinity    affinity,
    int         resonanceContribution,
    std::string passiveTrait)
    : Unit{ std::move(id), std::move(name), stats, affinity,
            resonanceContribution, std::move(passiveTrait) }
{
}

PlayableCharacter::~PlayableCharacter() = default;   // needs full IAction type here

void PlayableCharacter::addAbility(std::unique_ptr<IAction> action)
{
    m_abilities.push_back(std::move(action));
}

const std::vector<std::unique_ptr<IAction>>& PlayableCharacter::getAbilities() const
{
    return m_abilities;
}

int  PlayableCharacter::getSp()         const { return m_sp; }
int  PlayableCharacter::getEnergy()     const { return m_energy; }
bool PlayableCharacter::ultimateReady() const { return m_energy >= kMaxEnergy; }

void PlayableCharacter::gainSp(int amount)
{
    m_sp = std::min(kMaxSp, m_sp + amount);
}

void PlayableCharacter::useSp(int amount)
{
    m_sp = std::max(0, m_sp - amount);
}

void PlayableCharacter::gainEnergy(int amount)
{
    m_energy = std::min(kMaxEnergy, m_energy + amount);
}

void PlayableCharacter::resetEnergy()
{
    m_energy = 0;
}

void PlayableCharacter::displayActionMenu() const
{
    std::cout << "\n[" << m_name << "]"
        << "  SP: " << m_sp << '/' << kMaxSp
        << "  Energy: " << m_energy << '/' << kMaxEnergy << '\n';
    std::cout << "Actions:\n";
    for (std::size_t i{ 0 }; i < m_abilities.size(); ++i)
    {
        std::cout << "  [" << (i + 1) << "] " << m_abilities[i]->label();
        if (!m_abilities[i]->isAvailable(*this))
            std::cout << " [UNAVAILABLE]";
        std::cout << '\n';
    }
}

std::size_t PlayableCharacter::selectActionIndex()
{
    while (true)
    {
        std::cout << "Choose action: ";
        int choice{};
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::size_t idx{ static_cast<std::size_t>(choice - 1) };
        if (idx >= m_abilities.size()) { std::cout << "Invalid choice.\n"; continue; }
        if (!m_abilities[idx]->isAvailable(*this)) { std::cout << "Action unavailable.\n"; continue; }
        return idx;
    }
}

std::optional<TargetInfo> PlayableCharacter::selectTarget(const Party& enemies)
{
    std::vector<std::pair<std::size_t, const Unit*>> targets{};
    for (std::size_t i{ 0 }; i < enemies.size(); ++i)
    {
        const Unit* u = enemies.getUnitAt(i);
        if (u && u->isAlive())
            targets.emplace_back(i, u);
    }

    if (targets.empty())
        return std::nullopt;

    if (targets.size() == 1)
        return TargetInfo{ TargetInfo::Type::Enemy, targets[0].first };

    std::cout << "Choose target:\n";
    for (std::size_t i{ 0 }; i < targets.size(); ++i)
        std::cout << "  [" << (i + 1) << "] " << targets[i].second->getName() << '\n';

    while (true)
    {
        std::cout << "Target: ";
        int choice{};
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::size_t idx{ static_cast<std::size_t>(choice - 1) };
        if (idx >= targets.size()) { std::cout << "Invalid target.\n"; continue; }
        return TargetInfo{ TargetInfo::Type::Enemy, targets[idx].first };
    }
}

ActionResult PlayableCharacter::takeTurn(Party& allies, Party& enemies)
{
    displayActionMenu();
    std::size_t actionIdx = selectActionIndex();
    std::optional<TargetInfo> target = selectTarget(enemies);

    return m_abilities[actionIdx]->execute(*this, allies, enemies, target);
}