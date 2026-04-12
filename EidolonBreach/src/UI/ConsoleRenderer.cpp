#include "UI/ConsoleRenderer.h"
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include <iostream>

void ConsoleRenderer::printBar(int current, int maximum, int width)
{
    int filled{ (maximum > 0) ? (current * width / maximum) : 0 };
    std::cout << '[';
    for (int i{ 0 }; i < width; ++i)
        std::cout << ((i < filled) ? '#' : '.');
    std::cout << "] " << current << '/' << maximum;
}

// ── renderAttack ─────────────────────────────────────────────────────────
void ConsoleRenderer::renderAttack(const std::string& actorName, const ActionResult& result)
{
    if (!result.flavorText.empty())
        std::cout << result.flavorText << '\n';

    switch (result.type)
    {
    case ActionResult::Type::Damage:
        std::cout << actorName << " attacks for " << result.value << " damage!\n";
        break;
    case ActionResult::Type::Heal:
        std::cout << actorName << " regenerates " << result.value << " HP!\n";
        break;
    case ActionResult::Type::Charge:
        std::cout << actorName << " is charging up!\n";
        break;
    case ActionResult::Type::Skip:
        // Battle calls renderStunned for Skip results; nothing to print here.
        break;
    }
}

void ConsoleRenderer::renderBreak(const std::string& enemyName)
{
    std::cout << ">> BREAK! " << enemyName << " is stunned! <<\n";
}

void ConsoleRenderer::renderStunned(const std::string& enemyName)
{
    std::cout << enemyName << " is stunned and cannot act.\n";
}

void ConsoleRenderer::renderVictory(const std::string& enemyName, std::optional<Drop> drop)
{
    std::cout << '\n' << enemyName << " is destroyed!\n";
    if (drop.has_value())
        std::cout << "Loot: " << drop->name << " (" << drop->goldValue << " gold)\n";
    std::cout << "=== VICTORY ===\n";
}

void ConsoleRenderer::renderDefeat(const std::string& playerName)
{
    std::cout << '\n' << playerName << " has been defeated.\n";
    std::cout << "=== DEFEAT ===\n";
}

// ── printPartyStatus ─────────────────────────────────────────────────────
void ConsoleRenderer::printPartyStatus(const Party& playerParty, const Party& enemyParty)
{
    std::cout << "\n====================================\n";

    // Enemy side
    for (std::size_t i = 0; i < enemyParty.size(); ++i)
    {
        const Unit* u = enemyParty.getUnitAt(i);
        if (!u) continue;
        std::cout << "  [E" << (i + 1) << "] " << u->getName() << '\n';
        std::cout << "    HP:        "; printBar(u->getHp(), u->getMaxHp()); std::cout << '\n';

        const auto* e = dynamic_cast<const Enemy*>(u);
        if (e)
        {
            std::cout << "    Toughness: ";
            printBar(e->getToughness(), e->getMaxToughness());
            if (e->isBroken()) std::cout << " [BROKEN]";
            std::cout << '\n';
        }
    }

    std::cout << "  --\n";

    // Player side
    for (std::size_t i = 0; i < playerParty.size(); ++i)
    {
        const Unit* u = playerParty.getUnitAt(i);
        if (!u) continue;
        std::cout << "  [P" << (i + 1) << "] " << u->getName() << '\n';
        std::cout << "    HP:     "; printBar(u->getHp(), u->getMaxHp()); std::cout << '\n';

        const auto* pc = dynamic_cast<const PlayableCharacter*>(u);
        if (pc)
        {
            std::cout << "    SP:     " << pc->getSp() << '/' << PlayableCharacter::kMaxSp << '\n';
            std::cout << "    Energy: ";
            printBar(pc->getEnergy(), PlayableCharacter::kMaxEnergy, 10);
            std::cout << '\n';
        }
    }
    std::cout << "====================================\n";
}