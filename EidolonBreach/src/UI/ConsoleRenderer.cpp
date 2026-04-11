#include "ConsoleRenderer.h"
#include <iostream>

void ConsoleRenderer::renderAttack(const std::string& actorName, const ActionResult& result)
{
    // Print custom flavor text first if present
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
    std::cout << "\n" << enemyName << " is destroyed!\n";
    if (drop.has_value())
        std::cout << "Loot: " << drop->name << " (" << drop->goldValue << " gold)\n";
    std::cout << "=== VICTORY ===\n";
}
void ConsoleRenderer::renderDefeat(const std::string& playerName)
{
    std::cout << "\n" << playerName << " has been defeated.\n";
    std::cout << "=== DEFEAT ===\n";
}