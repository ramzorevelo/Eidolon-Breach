#pragma once

/**
 * @file PlayableCharacter.h
 * @brief Player‑controlled Unit with Energy and action abilities.
 */

#include "Actions/IAction.h"
#include "Core/ResourceStats.h"
#include "Entities/Unit.h"
#include <limits>
#include <memory>
#include <vector>
#include <UI/IInputHandler.h>
class IRenderer; // forward declaration — full definition in PlayableCharacter.cpp
class PlayableCharacter : public Unit
{
  public:
    static constexpr int kMaxMomentum{100}; 
    static constexpr int kArchSkillThreshold{40}; // Arch Skill available when momentum >= 40
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
    int getMomentum() const
    {
        return m_resources.momentum;
    }
    bool isUltimateReady() const
    {
        return m_resources.momentum >= kMaxMomentum;
    }
    bool isArchSkillReady() const
    {
        return m_resources.momentum >= kArchSkillThreshold;
    }
    void gainMomentum(int amount);
    void resetMomentum();

    // Delegates SP operations to the Party (shared pool)
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
    /** @brief Deduct amount from Momentum, clamped to 0. */
    void consumeMomentum(int amount);

    // Exposure gauge 
    static constexpr int kMaxExposure{100};
    static constexpr int kVentThreshold50{50}; // consolation proc threshold

    int getExposure() const
    {
        return m_exposure;
    }
    void modifyExposure(int delta);
    bool canVent() const; // 0 < exposure < 100
  private:
    std::vector<std::unique_ptr<IAction>> m_abilities;
    ResourceStats m_resources{0, kMaxMomentum};

    int m_resonanceContribution{};
    std::string m_passiveTrait{};

    void displayActionMenu(const Party &party, IRenderer &renderer) const;
    std::size_t selectActionIndex(const Party &allies, IInputHandler &input);
    std::optional<TargetInfo> selectTarget(const Party &enemies, IInputHandler &input);
    int m_exposure{0};
};