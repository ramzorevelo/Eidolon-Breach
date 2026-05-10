#pragma once
/**
 * @file BattleNode.h
 * @brief Map node that triggers a combat encounter.
 *        Enemy party is populated by a factory function supplied at construction.
 */

#include "Core/Affinity.h"
#include "Dungeon/MapNode.h"
#include <functional>

class Party;
class SummonRegistry;

class BattleNode : public MapNode
{
  public:
    /**
     * @param populateEnemies  Function called with an empty Party to fill it with enemies.
     * @param floorAffinity    Dominant affinity for this floor (applied to toughness).
     * @param xpReward         XP awarded to each alive PC on victory (Classic mode).
     */
    explicit BattleNode(std::function<void(Party &)> populateEnemies,
                        Affinity floorAffinity = Affinity::Aether,
                        int dungeonEnemyLevel = 1,
                        const SummonRegistry *summonRegistry = nullptr);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus,
               IRenderer &renderer, IInputHandler &input) override;

    [[nodiscard]] std::string description() const override;
    [[nodiscard]] NodeType nodeType() const override
    {
        return NodeType::Battle;
    }
  protected:
    /**
     * @brief Apply floor-affinity toughness modifiers to all enemies in the party.
     *        Matching affinity: +10% maxToughness. Opposing affinity: -10%.
     */
    void applyFloorAffinityModifiers(Party &enemyParty) const;

    /**
     * @brief Run the battle without displaying an entry prompt.
     *        Call this from subclasses that show their own prompt before combat.
     */
    void runBattle(Party &party, MetaProgress &meta,
                   RunContext &runCtx, EventBus &eventBus,
                   IRenderer &renderer, IInputHandler &input);

    void awardBattleXp(Party &party, MetaProgress &meta, const RunContext &runCtx) const;
  private:
    std::function<void(Party &)> m_populateEnemies;
    const SummonRegistry *m_summonRegistry{nullptr};
    Affinity m_floorAffinity{Affinity::Aether};
    int m_dungeonEnemyLevel{1};
};