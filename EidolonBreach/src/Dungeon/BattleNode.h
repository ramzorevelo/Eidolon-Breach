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
class ItemRegistry;

class BattleNode : public MapNode
{
  public:
    /**
     * @param populateEnemies  Function called with an empty Party to fill it with enemies.
     * @param floorAffinity    Dominant affinity for this floor (applied to toughness).
     * @param xpReward         XP awarded to each alive PC on victory (Classic mode).
     * @param floorIndex       0-based floor index used for Exposure depth modifier.
     * @param itemRegistry     Used in collectDrops() to resolve item-type drops.
     */
    explicit BattleNode(std::function<void(Party &)> populateEnemies,
                        Affinity floorAffinity = Affinity::Aether,
                        int dungeonEnemyLevel = 1,
                        const SummonRegistry *summonRegistry = nullptr,
                        int floorIndex = 0,
                        const ItemRegistry *itemRegistry = nullptr);

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
     * @brief Apply the floor-depth Exposure modifier to all alive PCs.
     *        Applies floorIndex * kFloorDepthExposureModifier to each PC.
     *        Call before battle.run() so Exposure is set at battle start.
     */
    void applyFloorExposureModifier(Party &party) const;

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
    int m_floorIndex{0};
    const ItemRegistry *m_itemRegistry{nullptr};
};