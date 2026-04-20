#pragma once
/**
 * @file PlayableCharacter.h
 * @brief Player-controlled Unit with Energy and action abilities.
 */

#include "Actions/IAction.h"
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

    // Arch Skill availability: cooldown-based 
    bool isArchSkillReady() const;

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

    void displayActionMenu(const Party &party, IRenderer &renderer) const;
    std::size_t selectActionIndex(const Party &allies, IInputHandler &input);
    std::optional<TargetInfo> selectTarget(const Party &enemies, IInputHandler &input);
};