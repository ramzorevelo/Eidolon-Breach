#include "UI/ConsoleRenderer.h"
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Core/IStatusEffect.h"
#include "Battle/ResonanceField.h"
#include <iostream>

void ConsoleRenderer::printBar(int current, int maximum, int width)
{
    int filled{ (maximum > 0) ? (current * width / maximum) : 0 };
    std::cout << '[';
    for (int i{ 0 }; i < width; ++i)
        std::cout << ((i < filled) ? '#' : '.');
    std::cout << "] " << current << '/' << maximum;
}

void ConsoleRenderer::renderActionResult(const std::string& actorName, const ActionResult& result)
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

void ConsoleRenderer::renderEffects(const Unit &unit)
{
    const auto &effects{unit.getEffects()};
    if (effects.empty())
        return;

    std::cout << "    Effects: ";
    for (const auto &effect : effects)
    {
        std::cout << effect->getName();
        if (effect->getDuration().has_value())
            std::cout << '(' << *effect->getDuration() << ')';
        else
            std::cout << "(perm)";
        std::cout << ' ';
    }
    std::cout << '\n';
}

void ConsoleRenderer::renderParty(const Party &party, const std::string &prefix, bool showToughness)
{
    if (prefix == "P")
    {
        // Show shared SP once at top of player section
        std::cout << "  Party SP: " << party.getSp() << '/' << party.getMaxSp() << '\n';
    }
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        const Unit *u = party.getUnitAt(i);
        if (!u)
            continue;
        std::string label{"[" + prefix + std::to_string(i + 1) + "]"};
        renderUnit(u, label, showToughness);
    }
}

void ConsoleRenderer::renderUnit(const Unit *unit, const std::string &label, bool showToughness)
{
    std::cout << "  " << label << " " << unit->getName() << '\n';
    std::cout << "    HP:        ";
    printBar(unit->getHp(), unit->getMaxHp());
    std::cout << '\n';

    renderEffects(*unit);

    if (showToughness)
    {
        const auto *e{dynamic_cast<const Enemy *>(unit)};
        if (e)
        {
            std::cout << "    Toughness: ";
            printBar(e->getToughness(), e->getMaxToughness());
            if (e->isBroken())
                std::cout << " [BROKEN]";
            std::cout << '\n';
        }
    }
    else
    {
        const auto *pc{dynamic_cast<const PlayableCharacter *>(unit)};
        if (pc)
        {
            // Energy only; SP is now shown at party level
            std::cout << "    Energy: ";
            printBar(pc->getMomentum(), PlayableCharacter::kMaxMomentum, 10);
            std::cout << '\n';
        }
    }
}


void ConsoleRenderer::renderPartyStatus(const Party& playerParty, const Party& enemyParty)
{
    std::cout << "\n====================================\n";

    renderParty(enemyParty, "E", /*showToughness=*/true);
    std::cout << "  --\n";
    renderParty(playerParty, "P", /*showToughness=*/false);

    std::cout << "====================================\n";
}

void ConsoleRenderer::renderMessage(const std::string &message)
{
    std::cout << message << '\n';
}

void ConsoleRenderer::renderResonanceField(const ResonanceField &field)
{
    std::cout << "  [Resonance Field] " << field.getGauge() << "/100";
    std::string summary{field.getVoteSummary()};
    if (!summary.empty())
        std::cout << " (" << summary << ')';
    std::cout << '\n';
}