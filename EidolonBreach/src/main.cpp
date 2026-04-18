#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Battle/Battle.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/StoneGolem.h"
#include "Entities/VampireBat.h"
#include <iostream>
#include <memory>

int main()
{
    std::cout << "=== 2v2 TEST: STRIKER + CONDUIT vs STONE GOLEM + VAMPIRE BAT ===\n";
    std::cout << "Goal: Basic attacks alone will lose. Use skills/ultimates to win.\n\n";

    Party playerParty;
    // Shared SP starts at 50 (per spec §2.3.2)
    playerParty.gainSp(50);

    auto striker = std::make_unique<PlayableCharacter>(
        "striker_1", "Striker",
        Stats{80, 80, 22, 5, 14},
        Affinity::Blaze,
        10);
    striker->addAbility(std::make_unique<BasicStrikeAction>());
    striker->addAbility(std::make_unique<SkillAction>(2.0f));
    striker->addAbility(std::make_unique<UltimateAction>());
    playerParty.addUnit(std::move(striker));

    auto conduit = std::make_unique<PlayableCharacter>(
        "conduit_1", "Conduit",
        Stats{110, 110, 10, 15, 9},
        Affinity::Terra,
        8);
    conduit->addAbility(std::make_unique<BasicStrikeAction>());
    conduit->addAbility(std::make_unique<SkillAction>(2.0f));
    conduit->addAbility(std::make_unique<UltimateAction>());
    playerParty.addUnit(std::move(conduit));

    Party enemyParty;
    enemyParty.addUnit(std::make_unique<StoneGolem>("Stone Golem", 130, 60));
    enemyParty.addUnit(std::make_unique<VampireBat>("Vampire Bat", 100, 40));

    Battle battle{playerParty, enemyParty};
    battle.run();

    return 0;
}