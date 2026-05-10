/**
 * @file BondTrial.cpp
 * @brief BondTrial implementation. Per-character enemy setup and reward.
 */
#include "Meta/BondTrial.h"
#include "Battle/Battle.h"
#include "Entities/IAIStrategy.h"
#include "Characters/CharacterRegistry.h"
#include "Characters/Lyra/Lyra.h"
#include "Characters/Vex/Vex.h"
#include "Characters/Zara/Zara.h"
#include "Core/CombatConstants.h"
#include "Core/EventBus.h"
#include "Core/MetaProgress.h"
#include "Core/RunContext.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Summons/SummonRegistry.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <memory>

namespace
{
std::unique_ptr<Enemy> makeBondEnemy(std::string_view characterId)
{
    // Lyra faces a high-Toughness Tempest enemy to test Blaze break pressure.
    // Vex faces a Terra enemy to test sustained damage.
    // Zara faces a Blaze enemy with high ATK to test Frost Slow.
    if (characterId == LyraIds::kId)
        return std::make_unique<Enemy>(
            "bond_guardian_lyra", "Flame Guardian",
            Stats{150, 150, 18, 10, 10}, Affinity::Tempest, 60,
            std::make_unique<BasicAIStrategy>(),
            std::map<Affinity, float>{{Affinity::Blaze, 2.0f}});

    if (characterId == VexIds::kId)
        return std::make_unique<Enemy>(
            "bond_guardian_vex", "Terra Sentinel",
            Stats{180, 180, 15, 14, 8}, Affinity::Terra, 80,
            std::make_unique<BasicAIStrategy>(),
            std::map<Affinity, float>{{Affinity::Aether, 2.0f}});

    if (characterId == ZaraIds::kId)
        return std::make_unique<Enemy>(
            "bond_guardian_zara", "Blaze Striker",
            Stats{140, 140, 22, 8, 12}, Affinity::Blaze, 55,
            std::make_unique<BasicAIStrategy>(),
            std::map<Affinity, float>{{Affinity::Frost, 2.0f}});

    return std::make_unique<Enemy>(
        "bond_guardian_generic", "Guardian",
        Stats{160, 160, 16, 12, 10}, Affinity::Aether, 70,
        std::make_unique<BasicAIStrategy>());
}
} // namespace

bool BondTrial::run(std::string_view characterId,
                    MetaProgress &meta,
                    const CharacterRegistry &charRegistry,
                    const AbilityRegistry & /*abilityRegistry*/,
                    const SummonRegistry &summonRegistry,
                    SDL3Renderer &renderer,
                    SDL3InputHandler &input)
{
    const std::string id{characterId};

    if (meta.characterInsight.count(id) &&
        meta.characterInsight.at(id).bondTrialComplete)
    {
        renderer.renderMessage("Bond Trial already complete for " + id + ".");
        return false;
    }

    const int level{meta.characterLevels.count(id) ? meta.characterLevels.at(id) : 1};
    auto pc{charRegistry.create(characterId, level, &meta)};
    if (!pc)
    {
        renderer.renderMessage("Character not found: " + id);
        return false;
    }

    Party playerParty{};
    playerParty.addUnit(std::move(pc));

    Party enemyParty{};
    enemyParty.addUnit(makeBondEnemy(characterId));

    RunContext ctx{};
    EventBus bus{};

    Battle battle{playerParty, enemyParty, renderer, input, ctx, bus,
                  nullptr, &summonRegistry};
    battle.run();
    const bool won{!playerParty.isAllDead()};

    if (won)
    {
        meta.characterInsight[id].bondTrialComplete = true;
        meta.characterInsight[id].insightBalance += CombatConstants::kBondTrialInsightBonus;
        renderer.renderMessage("Bond Trial complete! +" +
                               std::to_string(CombatConstants::kBondTrialInsightBonus) +
                               " Insight awarded.");
    }

    return won;
}