#pragma once

/** 
 * @file ConsoleRenderer.h 
 * @brief Simple console output renderer (implements IRenderer in Phase 3).
 */

#include "Core/ActionResult.h"
#include "Core/Drop.h"
#include <string>
#include <optional>

class Party;
class Unit;

/** Static renderer for console output. */
class ConsoleRenderer
{
public:
    static void renderAttack(const std::string& actorName, const ActionResult& result);
    static void renderBreak(const std::string& enemyName);
    static void renderStunned(const std::string& enemyName);
    static void renderVictory(const std::string& enemyName, std::optional<Drop> drop);
    static void renderDefeat(const std::string& playerName);
    /**
     * @brief Print the name and remaining duration of each active effect on a unit.
     * @param unit The unit whose effects to render.
     */
    static void renderEffects(const Unit &unit);
    static void printPartyStatus(const Party& playerParty, const Party& enemyParty);

private:
    static void printBar(int current, int maximum, int width = 20);
    static void renderParty(const Party& party, const std::string& prefix, bool showToughness);
    static void renderUnit(const Unit* unit, const std::string& label, bool showToughness);
};