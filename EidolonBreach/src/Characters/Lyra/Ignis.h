#pragma once
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include "Summons/SummonDefinition.h"
#include "Summons/SummonRegistry.h"
#include "Entities/Summon.h" 
#include <algorithm>
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
        [](Summon &ignis, Party & /*allies*/, Party &enemies, BattleState & /*state*/)
        {
            const auto alive{enemies.getAliveUnits()};
            if (alive.empty())
                return ActionResult{ActionResult::Type::Skip, 0};

            Unit *target{alive.front()};
            // Damage = floor(summonerAtk * 0.6), minimum 1.
            const int damage{std::max(1, static_cast<int>(
                                             static_cast<float>(ignis.getSummonerAtk()) * 0.6f))};
            target->takeTrueDamage(damage);

            ActionResult result{ActionResult::Type::Damage, damage};
            result.flavorText = ">> Ignis scorches the enemy! <<";
            result.actionAffinity = Affinity::Blaze;
            return result;
        }});

    registry.registerDefinition(std::move(def));
}
} // namespace Ignis