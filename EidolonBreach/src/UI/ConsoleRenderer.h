#pragma once
#include "Core/ActionResult.h"
#include "Core/Drop.h"
#include <string>
#include <optional>

class ConsoleRenderer
{
public:
    static void renderAttack(const std::string& actorName, const ActionResult& result);
    static void renderBreak(const std::string& enemyName);
    static void renderStunned(const std::string& enemyName);
    static void renderVictory(const std::string& enemyName, std::optional<Drop> drop);
    static void renderDefeat(const std::string& playerName);
};