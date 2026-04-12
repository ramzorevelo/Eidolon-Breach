#pragma once
#include "Entities/Unit.h"
#include "Actions/IAction.h"
#include <vector>
#include <memory>
#include <limits>

/** 
 * @file PlayableCharacter.h
 * @brief Player‑controlled Unit with SP/Energy and action abilities.
 */
class PlayableCharacter : public Unit
{
public:
    static constexpr int kMaxSp{ 5 };
    static constexpr int kMaxEnergy{ 100 };

    PlayableCharacter(std::string id,
        std::string name,
        Stats       stats,
        Affinity    affinity,
        int         resonanceContribution,
        std::string passiveTrait = "");

    // Declare destructor so unique_ptr<IAction> can be incomplete in the header.
    ~PlayableCharacter() override;

    void addAbility(std::unique_ptr<IAction> action);
    const std::vector<std::unique_ptr<IAction>>& getAbilities() const;

    int  getSp()         const;
    int  getEnergy()     const;
    bool ultimateReady() const;

    void gainSp(int amount);
    void useSp(int amount);      // subtracts; actions call this (e.g., SkillAction)
    void gainEnergy(int amount);
    void resetEnergy();          // UltimateAction calls this

    /** Displays action menu, reads player input, executes chosen action. */
    ActionResult takeTurn(Party& allies, Party& enemies) override;

private:
    std::vector<std::unique_ptr<IAction>> m_abilities;
    int m_sp{ 3 };
    int m_energy{ 0 };
};