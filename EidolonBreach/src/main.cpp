#include "Battle/Battle.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/VampireBat.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include <iostream>
#include <memory>
#include <string>

int main()
{
    std::cout << "Enter your hero's name: ";
    std::string name{};
    std::getline(std::cin, name);
    if (name.empty()) name = "Hero";

    // ── Player party ──────────────────────────────────────────────────
    Party playerParty;

    // Generic hero (Aether, balanced stats, SPD 10)
    auto hero = std::make_unique<PlayableCharacter>(
        "hero_1", name,
        Stats{ 120, 120, 15, 10, 10 },
        Affinity::Aether, 10);
    hero->addAbility(std::make_unique<BasicStrikeAction>());
    hero->addAbility(std::make_unique<SkillAction>());
    hero->addAbility(std::make_unique<UltimateAction>());
    playerParty.addUnit(std::move(hero));


    // ── Enemy party ───────────────────────────────────────────────────
    Party enemyParty;
    enemyParty.addUnit(std::make_unique<VampireBat>("Vampire Bat", 100, 40));

    // ── Run ───────────────────────────────────────────────────────────
    Battle battle{ playerParty, enemyParty };
    battle.run();

    return 0;
}