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

    // Delegates SP operations to the Party (shared pool)
    bool canAffordSp(int amount, const Party &party) const;
    void consumeSp(int amount, Party &party) const;

    /** Displays action menu, reads player input, executes chosen action. */
    ActionResult takeTurn(Party &allies, Party &enemies) override;

    int getResonanceContribution() const
    {
        return m_resonanceContribution;
    }
    const std::string &getPassiveTrait() const
    {
        return m_passiveTrait;
    }

  private:
    std::vector<std::unique_ptr<IAction>> m_abilities;
    ResourceStats m_resources{0, kMaxEnergy};

    int m_resonanceContribution{};
    std::string m_passiveTrait{};

    void displayActionMenu(const Party &party) const;
    std::size_t selectActionIndex(const Party &party);
    std::optional<TargetInfo> selectTarget(const Party &enemies);
};