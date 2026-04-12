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

/** Displays action menu, reads player input, executes chosen action. */
ActionResult PlayableCharacter::takeTurn(Party& allies, Party& enemies)
{
    std::vector<std::pair<std::size_t, Unit*>> targets;
    for (std::size_t i = 0; i < enemies.size(); ++i)
    {
        Unit* u = enemies.getUnitAt(i);
        if (u && u->isAlive())
            targets.emplace_back(i, u);
    }

    // ── Action menu ──────────────────────────────────────────────────
    std::cout << "\n[" << m_name << "]"
        << "  SP: " << m_sp << '/' << kMaxSp
        << "  Energy: " << m_energy << '/' << kMaxEnergy << '\n';
    std::cout << "Actions:\n";
    for (std::size_t i = 0; i < m_abilities.size(); ++i)
    {
        std::cout << "  [" << (i + 1) << "] " << m_abilities[i]->label();
        if (!m_abilities[i]->isAvailable(*this))
            std::cout << " [UNAVAILABLE]";
        std::cout << '\n';
    }

    // ── Pick action ──────────────────────────────────────────────────
    std::size_t actionIdx = 0;
    while (true)
    {
        std::cout << "Choose action: ";
        int choice;
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::size_t idx = static_cast<std::size_t>(choice - 1);
        if (idx >= m_abilities.size()) { std::cout << "Invalid choice.\n";     continue; }
        if (!m_abilities[idx]->isAvailable(*this)) { std::cout << "Action unavailable.\n"; continue; }
        actionIdx = idx;
        break;
    }

    // ── Pick target (Phase 1: always targets an enemy) ───────────────
    std::optional<TargetInfo> target = std::nullopt;
    if (!targets.empty())
    {
        std::size_t partyIdx = targets[0].first;   // default: auto-select the only target

        if (targets.size() > 1)
        {
            std::cout << "Choose target:\n";
            for (std::size_t i = 0; i < targets.size(); ++i)
                std::cout << "  [" << (i + 1) << "] " << targets[i].second->getName() << '\n';

            while (true)
            {
                std::cout << "Target: ";
                int choice;
                if (!(std::cin >> choice))
                {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input.\n";
                    continue;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::size_t idx = static_cast<std::size_t>(choice - 1);
                if (idx >= targets.size()) { std::cout << "Invalid target.\n"; continue; }
                partyIdx = targets[idx].first;
                break;
            }
        }
        target = TargetInfo{ TargetInfo::Type::Enemy, partyIdx };
    }

    return m_abilities[actionIdx]->execute(*this, allies, enemies, target);
}