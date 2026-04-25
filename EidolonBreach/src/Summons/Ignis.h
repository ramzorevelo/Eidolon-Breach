#pragma once
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include "Summons/SummonDefinition.h"
#include "Summons/SummonRegistry.h"

class Summon;
struct BattleState;

namespace Ignis
{
constexpr std::string_view kId{"ignis"};

inline void registerIgnis(SummonRegistry &registry)
{
    SummonDefinition def{};
    def.id = std::string{kId};
    def.displayName = "Ignis";
    def.baseStats = Stats{40, 40, 0, 0, 90};
    def.duration = std::nullopt;

    def.actions.push_back(SummonAction{
        "Ember Burst",
        [](Summon & /*ignis*/ , Party & /*allies*/, Party &enemies, BattleState & /*state*/)
        {
            const auto alive{enemies.getAliveUnits()};
            if (alive.empty())
                return ActionResult{ActionResult::Type::Skip, 0};

            Unit *target{alive.front()};
            constexpr int kIgnisBaseDamage{9};
            target->takeTrueDamage(kIgnisBaseDamage);

            ActionResult result{ActionResult::Type::Damage, kIgnisBaseDamage};
            result.flavorText = ">> Ignis scorches the enemy! <<";
            result.actionAffinity = Affinity::Blaze;
            return result;
        }});

    registry.registerDefinition(std::move(def));
}
} // namespace Ignis