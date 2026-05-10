#pragma once
/**
 * @file PlayableCharacter.h
 * @brief Player-controlled Unit with Energy and action abilities.
 */

#include "Actions/IAction.h"
#include "Actions/SlotState.h"
#include "Core/ResourceStats.h"
#include "Core/CharacterMod.h"
#include "Entities/Unit.h"
#include "Items/Item.h"
#include "UI/IInputHandler.h"
#include "Core/CombatConstants.h"
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
    /**
     * @brief Set the character's archetype string. Called once by CharacterRegistry::create.
     */
    void setArchetype(std::string_view arch)
    {
        m_archetype = std::string{arch};
    }
    void setFractureSelfDotPct(float pct)
    {
        m_fractureSelfDotPct = pct;
    }
    [[nodiscard]] float fractureSelfDotPct() const
    {
        return m_fractureSelfDotPct;
    }

    void setBreachbornActionBonus(float divisor, int burnDamage, int burnDuration)
    {
        m_breachbornActionBonusDivisor = divisor;
        m_breachbornActionBurnDamage = burnDamage;
        m_breachbornActionBurnDuration = burnDuration;
    }
    [[nodiscard]] float breachbornActionBonusDivisor() const
    {
        return m_breachbornActionBonusDivisor;
    }
    [[nodiscard]] int breachbornActionBurnDamage() const
    {
        return m_breachbornActionBurnDamage;
    }
    [[nodiscard]] int breachbornActionBurnDuration() const
    {
        return m_breachbornActionBurnDuration;
    }

    /**
     * @brief Permanently apply a CharacterMod to this character.
     *        StatBonus modifies base stats and resets HP to the new max.
     *        All other mods update the corresponding override member.
     *        Call once per activated node during CharacterRegistry::create().
     */
    void applyCharacterMod(const CharacterMod &mod);

    /** @return Effective arch skill cooldown after CooldownReduction mods. */
    [[nodiscard]] int effectiveArchSkillCooldown() const;

    /** @return Proc damage/duration multiplier from ProcEnhancement nodes. */
    [[nodiscard]] float procEnhancementMultiplier() const
    {
        return m_procEnhancementMultiplier;
    }

    const std::vector<std::unique_ptr<IAction>> &getAbilities() const;

    /**
     * @brief Unlocks slot `slotIndex` (0 or 1) if not already unlocked.
     * Called by MetaProgress::gainXP() when a level threshold is crossed.
     * No-op if slotIndex is out of range or already unlocked.
     */
    void tryUnlockSlot(int slotIndex);

    /**
     * @brief Set the raw IAction* observer for a slot.
     *        The pointer must point into m_abilities (owned there).
     */
    void equipSkillToSlot(int slotIndex, IAction *skill);


    /** @return Read-only view of the equipped skill set. */
    const EquippedSkillSet &getEquippedSkills() const
    {
        return m_equipped;
    }

    [[nodiscard]] int getEnergy() const
    {
        return m_resources.energy;
    }
    [[nodiscard]] bool isUltimateReady() const
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

    [[nodiscard]] bool isArchSkillUnlocked() const
    {
        return hasFlag(CharFlag::ArchSkillUnlocked);
    }

    /**
     * @brief Apply all level-based unlocks for this character.
     *        Call at creation (with stored level) and after each level-up.
     * @param level The character's current level.
     */
    void applyUnlocks(int level);


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

    [[nodiscard]] int getResonanceContribution() const override
    {
        return m_resonanceContribution;
    }
    [[nodiscard]] const std::string &getPassiveTrait() const
    {
        return m_passiveTrait;
    }

    [[nodiscard]] const std::string &getArchetype() const
    {
        return m_archetype;
    }

    // Exposure gauge
    static constexpr int kMaxExposure{100};
    static constexpr int kVentThreshold50{50};

    int getExposure() const
    {
        return m_exposure;
    }
    void modifyExposure(int delta);
    [[nodiscard]] bool canVent() const; // 0 < exposure < 100

    /**
     * @brief AV reset multiplier based on current Exposure state.
     *        Applied by AVTurnOrderCalculator when resetting this PC's AV.
     *        Summons and enemies always return kAvModBaseline from exposureModifier().
     */
    [[nodiscard]] float getExposureAVModifier() const;


    [[nodiscard]] bool isBreachbornActive() const
    {
        return hasFlag(CharFlag::BreachbornActive);
    }

    [[nodiscard]] bool isFractured() const
    {
        return hasFlag(CharFlag::Fractured);
    }

    /** @brief Directly applies Fracture state. Used by Battle after Breachborn ends,
     *         and by tests to set up Fracture scenarios without running a full battle. */
    void applyFracture()
    {
        setFlag(CharFlag::Fractured);
    }

    /**
     * @brief Activates Breachborn for 3 turns. Safe to call while Fractured
     *        (refreshes the counter without changing Fracture status).
     */
    void activateBreachborn()
    {
        setFlag(CharFlag::BreachbornActive);
        m_breachbornTurnsRemaining = CombatConstants::kBreachbornDurationTurns;
    }

    /**
     * @brief Decrements the Breachborn counter by one turn.
     *        When the counter reaches 0 Breachborn ends and Fracture activates.
     * @return true if Breachborn just ended and Fracture is now active.
     */
    bool tickBreachborn()
    {
        if (!hasFlag(CharFlag::BreachbornActive))
            return false;
        --m_breachbornTurnsRemaining;
        if (m_breachbornTurnsRemaining <= 0)
        {
            clearFlag(CharFlag::BreachbornActive);
            setFlag(CharFlag::Fractured);
            return true;
        }
        return false;
    }

    [[nodiscard]] int getBreachbornTurnsRemaining() const
    {
        return m_breachbornTurnsRemaining;
    }

    
    [[nodiscard]] bool isResonatingProcArmed() const
    {
        return hasFlag(CharFlag::ResonatingProcArmed);
    }
    void armResonatingProc()
    {
        setFlag(CharFlag::ResonatingProcArmed);
    }
    void consumeResonatingProc()
    {
        clearFlag(CharFlag::ResonatingProcArmed);
    }

    [[nodiscard]] bool isSurgingProcArmed() const
    {
        return hasFlag(CharFlag::SurgingProcArmed);
    }
    void armSurgingProc()
    {
        setFlag(CharFlag::SurgingProcArmed);
    }
    void consumeSurgingProc()
    {
        clearFlag(CharFlag::SurgingProcArmed);
    }

    // Consumable cooldown
    /** @return true when a consumable may be used this turn. */
    [[nodiscard]] bool canUseConsumable() const;

    /**
     * @brief Called when a consumable is used.
     * @param multiTurnEffect If true, the consumable is locked for the entire battle.
     */
    void consumeConsumableAction(bool multiTurnEffect = false);

    /** @brief Decrements m_consumableCooldown by 1, clamped to 0. Called at turn start. */
    void tickConsumableCooldown();

    /** @brief Resets m_consumableCooldown and m_consumableUsedThisBattle. Called at battle end. */
    void resetBattleConsumableState();

    /** @brief Resets arch skill cooldown to 0. Call at battle end. */
    void resetArchSkillCooldown();

    [[nodiscard]] int getConsumableCooldown() const
    {
        return m_consumableCooldown;
    }

    /**
     * @brief Per-character equipment loadout (weapon, armor, accessory).
     *        Each slot holds at most one Item. std::nullopt = unequipped.
     */
    struct CharacterEquipment
    {
        std::optional<Item> weapon{};
        std::optional<Item> armor{};
        std::optional<Item> accessory{};
    };
    // Equipment
    /**
     * @brief Equip an item into the appropriate slot.
     *        If the slot was occupied, the previous item is returned via the
     *        return value so the caller can return it to inventory.
     *        Sets ResonanceModifier on m_resonanceContribution immediately.
     * @return The displaced item, if any.
     */
    std::optional<Item> equip(const Item &item);

    /**
     * @brief Remove the item from the given slot and return it.
     *        Reverses any ResonanceModifier. Returns nullopt if slot was empty.
     */
    std::optional<Item> unequip(EquipSlot slot);

    /** @return Read-only view of all equipped items. */
    [[nodiscard]] const CharacterEquipment &getEquipment() const
    {
        return m_equipment;
    }

    /**
     * @brief Compute final stats: equipment pre-pass (flat StatModifiers from gear),
     *        then the two-pass IStatusEffect fold (flat then pct).
     *        Override of Unit::getFinalStats().
     */
    [[nodiscard]] Stats getFinalStats() const;
    [[nodiscard]] int getAffinityResistance(Affinity affinity) const override;

    void onBattleReset() override;
    void gainEnergyIfApplicable(int amount) override
    {
        gainEnergy(amount);
    }
    [[nodiscard]] PlayableCharacter *asPlayableCharacter() override
    {
        return this;
    }
  private:
    std::vector<std::unique_ptr<IAction>> m_abilities;
    ResourceStats m_resources{0, kMaxEnergy};

    int m_resonanceContribution{};
    std::string m_passiveTrait{};
    std::string m_archetype{};
    int m_exposure{0};

    EquippedSkillSet m_equipped{};

    void displayActionMenu(const Party &party, IRenderer &renderer) const;
    std::size_t selectActionIndex(const Party &allies, IInputHandler &input);

    /**
     * @brief Build a numbered enemy list, display it, prompt for choice.
     * @return TargetInfo with Type::Enemy, or nullopt if no enemies alive.
     */
    std::optional<TargetInfo> selectTarget(const Party &enemies,
                                           IInputHandler &input,
                                           IRenderer &renderer);

    /**
     * @brief Build a numbered ally list, display it, prompt for choice.
     * @return TargetInfo with Type::Ally, or nullopt if no allies alive.
     */
    std::optional<TargetInfo> selectAllyTarget(const Party &allies,
                                               IInputHandler &input,
                                               IRenderer &renderer);

    /**
     * @brief Shared targeting logic for both enemy and ally selection.
     * @param pool       Party to select from.
     * @param isAlly     true for ally targeting, false for enemy targeting.
     * @param input      Input handler reference.
     * @param renderer   Renderer reference.
     * @return TargetInfo with the appropriate type, or nullopt on cancel.
     */
    std::optional<TargetInfo> selectFromParty(const Party &pool,
                                              bool isAlly,
                                              IInputHandler &input,
                                              IRenderer &renderer);
    int m_archSkillCooldown{0};
    int m_consumableCooldown{0};
    int m_breachbornTurnsRemaining{0};
    CharacterEquipment m_equipment{};

    enum class CharFlag : uint16_t
    {
        None = 0,
        ArchSkillUnlocked = 1 << 0,
        ResonatingProcArmed = 1 << 1,
        SurgingProcArmed = 1 << 2,
        BreachbornActive = 1 << 3,
        Fractured = 1 << 4,
        ConsumableUsedThisBattle = 1 << 5,
    };

    static constexpr uint16_t kBattleResetFlagsMask =
        static_cast<uint16_t>(CharFlag::ResonatingProcArmed) |
        static_cast<uint16_t>(CharFlag::SurgingProcArmed) |
        static_cast<uint16_t>(CharFlag::BreachbornActive) |
        static_cast<uint16_t>(CharFlag::ConsumableUsedThisBattle);

    uint16_t m_flags{0};

    void setFlag(CharFlag f)
    {
        m_flags |= static_cast<uint16_t>(f);
    }
    void clearFlag(CharFlag f)
    {
        m_flags &= static_cast<uint16_t>(~static_cast<uint16_t>(f));
    }
    [[nodiscard]] bool hasFlag(CharFlag f) const
    {
        return (m_flags & static_cast<uint16_t>(f)) != 0;
    }
    float m_fractureSelfDotPct{0.0f};
    float m_breachbornActionBonusDivisor{0.0f};
    int m_breachbornActionBurnDamage{0};
    int m_breachbornActionBurnDuration{0};

    // CharacterMod overrides. Applied by CharacterRegistry::create()
    int m_cooldownReduction{0};
    int m_exposureThreshold50{CombatConstants::kExposureThreshold50};
    int m_exposureThreshold75{CombatConstants::kExposureThreshold75};
    int m_slotUnlockLevelOverride[2]{CombatConstants::kSlot1UnlockLevel,
                                     CombatConstants::kSlot2UnlockLevel};
    float m_procEnhancementMultiplier{1.0f};
    float m_avModBonusBaseline{0.0f};
    float m_avModBonusResonating{0.0f};
    float m_avModBonusSurging{0.0f};
    float m_avModBonusBreachborn{0.0f};
    float m_avModBonusFractured{0.0f};
};