#include "Battle/Battle.h"
#include "UI/ConsoleRenderer.h"
#include "Entities/Player.h"
#include "Entities/Enemy.h"
#include "Core/ActionResult.h"
#include <iostream>
#include <limits>

Battle::Battle(Player& player, Enemy& enemy, std::vector<std::unique_ptr<IAction>> actions)
    : m_player{ player }
    , m_enemy{ enemy }
    , m_actions{ std::move(actions) }
{
}

void Battle::printBar(int current, int maximum, int width) const
{
    int filled{ (maximum > 0) ? (current * width / maximum) : 0 };
    std::cout << '[';
    for (int i{ 0 }; i < width; ++i)
        std::cout << ((i < filled) ? '#' : '.');
    std::cout << "] " << current << '/' << maximum;
}

void Battle::printStatus() const
{
    std::cout << "\n------------------------------------\n";

    std::cout << "  " << m_enemy.getName() << '\n';
    std::cout << "  HP:        ";
    printBar(m_enemy.getHp(), m_enemy.getMaxHp(), 20);
    std::cout << '\n';
    std::cout << "  Toughness: ";
    printBar(m_enemy.getToughness(), m_enemy.getMaxToughness(), 20);
    std::cout << '\n';

    std::cout << "\n  " << m_player.getName() << '\n';
    std::cout << "  HP:        ";
    printBar(m_player.getHp(), m_player.getMaxHp(), 20);
    std::cout << '\n';
    std::cout << "  SP:     " << m_player.getSp()
        << '/' << Player::kMaxSp << '\n';
    std::cout << "  Energy: ";
    printBar(m_player.getEnergy(), Player::kMaxEnergy, 10);
    std::cout << '\n';

    std::cout << "------------------------------------\n";
}

void Battle::playerTurn()
{
    std::cout << "\nYour actions:\n";
    for (std::size_t i = 0; i < m_actions.size(); ++i)
    {
        std::cout << "  [" << (i + 1) << "] " << m_actions[i]->label();
        if (!m_actions[i]->isAvailable(m_player))
            std::cout << " [NOT AVAILABLE]";
        std::cout << '\n';
    }

    int choice{};
    while (true)
    {
        std::cout << "Choose: ";
        std::cin >> choice;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::size_t idx = static_cast<std::size_t>(choice - 1);
        if (idx >= m_actions.size())
        {
            std::cout << "Invalid choice.\n";
            continue;
        }

        if (!m_actions[idx]->isAvailable(m_player))
        {
            std::cout << "Action not available.\n";
            continue;
        }

        ActionResult result = m_actions[idx]->execute(m_player, m_enemy);
        ConsoleRenderer::renderAttack(m_player.getName(), result);
        break;
    }

    if (m_enemy.isBroken())
        ConsoleRenderer::renderBreak(m_enemy.getName());
}

void Battle::enemyTurn()
{
    if (m_enemy.isBroken())
    {
        ConsoleRenderer::renderStunned(m_enemy.getName());
        m_enemy.recoverFromBreak();
        return;
    }

    ActionResult result{ m_enemy.performAttack() };
    ConsoleRenderer::renderAttack(m_enemy.getName(), result);

    switch (result.type)
    {
    case ActionResult::Type::Damage:
        m_player.takeDamage(result.value);
        break;
    case ActionResult::Type::Heal:
        m_enemy.heal(result.value);
        break;
    default:
        break;
    }

    if (m_enemy.isBroken())
        ConsoleRenderer::renderBreak(m_enemy.getName());
}

void Battle::run()
{
    std::cout << "\n=== BATTLE START ===\n";
    std::cout << m_player.getName() << " vs " << m_enemy.getName() << '\n';

    while (m_player.isAlive() && m_enemy.isAlive())
    {
        printStatus();
        playerTurn();

        if (!m_enemy.isAlive())
        {
            auto drop{ m_enemy.dropLoot() };
            ConsoleRenderer::renderVictory(m_enemy.getName(), std::move(drop));
            return;
        }

        enemyTurn();

        if (!m_player.isAlive())
        {
            ConsoleRenderer::renderDefeat(m_player.getName());
            return;
        }
    }
}
