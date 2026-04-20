#pragma once
/**
 * @file PlayableCharacter.h
 * @brief Player-controlled Unit with Energy and action abilities.
 */

#include "Actions/IAction.h"
#include "Actions/SlotState.h"
#include "Core/ResourceStats.h"
#include "Entities/Unit.h"
#include "UI/IInputHandler.h"
#include <memory>
#include <optional>
#include <vector>
#include <ostream>
class IRenderer;

class PlayableCharacter : public Unit
{
  public:
    static constexpr int kMaxEnergy{100};

    PlayableCharacter(std::string id,
                      std::string name,
                      Stats stats,
                      Affinity affinity,
                      int resonanceContribution,
                      std::string passiveTrait = "");

    ~PlayableCharacter() override;

    void addAbility(std::unique_ptr<IAction> action);
    const std::vector<std::unique_ptr<IAction>> &getAbilities() const;

    /**
     * @brief Unlocks slot `slotIndex` (0 or 1) if not already unlocked.
     * Called by MetaProgress::gainXP() when a level threshold is crossed.
     * No-op if slotIndex is out of range or already unlocked.
     */
    void tryUnlockSlot(int slotIndex);

    /** @return Read-only view of the equipped skill set. */
    const EquippedSkillSet &getEquippedSkills() const
    {
        return m_equipped;
    }

    // Energy (individual resource)
    int getEnergy() const
    {
        return m_resources.energy;
    }
    bool isUltimateReady() const
    {
        return m_resources.energy >= kMaxEnergy;
    }
    void gainEnergy(int amount);
    void resetEnergy();
    void consumeEnergy(int amount);

    // Arch Skill cooldown
    /** @return true when m_archSkillCooldown == 0. */
    [[nodiscard]] bool isArchSkillReady() const
    {
        return m_archSkillCooldown == 0;
    }
    /** @return Remaining cooldown turns for UI display. */
    [[nodiscard]] int getArchSkillCooldown() const
    {
        return m_archSkillCooldown;
    }
    /** Sets cooldown to kArchSkillCooldownTurns. Call after Arch Skill is used. */
    void consumeArchSkill();
    /** Decrements cooldown by 1, clamped to 0. Call at the start of each PC turn. */
    void tickArchSkillCooldown();

    // SP: delegates to Party (shared pool)
    bool canAffordSp(int amount, const Party &party) const;
    void consumeSp(int amount, Party &party) const;

    /** Displays action menu, reads player input, executes chosen action. */
    ActionResult takeTurn(Party &allies, Party &enemies, BattleState &state) override;

    int getResonanceContribution() const
    {
        return m_resonanceContribution;
    }
    const std::string &getPassiveTrait() const
    {
        return m_passiveTrait;
    }

    // Exposure gauge
    static constexpr int kMaxExposure{100};
    static constexpr int kVentThreshold50{50};

    int getExposure() const
    {
        return m_exposure;
    }
    void modifyExposure(int delta);
    bool canVent() const; // 0 < exposure < 100

  private:
    std::vector<std::unique_ptr<IAction>> m_abilities;
    ResourceStats m_resources{0, kMaxEnergy};

    int m_resonanceContribution{};
    std::string m_passiveTrait{};
    int m_exposure{0};

    EquippedSkillSet m_equipped{};

    void displayActionMenu(const Party &party, IRenderer &renderer) const;
    std::size_t selectActionIndex(const Party &allies, IInputHandler &input);
    std::optional<TargetInfo> selectTarget(const Party &enemies, IInputHandler &input);
    int m_archSkillCooldown{0};
};