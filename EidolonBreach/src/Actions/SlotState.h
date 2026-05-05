#pragma once
/**
 * @file SlotState.h
 * @brief SlotState and EquippedSkillSet for PlayableCharacter equippable skill slots.
 */

#include <array>

class IAction;

/** State of one equippable skill slot. */
struct SlotState
{
    bool unlocked{false};
    IAction *equippedSkill{nullptr}; ///< Raw observer; owned by PlayableCharacter::m_abilities.

    /** @return true when the slot is both unlocked and has a skill equipped. */
    [[nodiscard]] bool isReady() const
    {
        return unlocked && equippedSkill != nullptr;
    }
};

/** Two-slot equip set for Slot Skills (keys 1 and 2). */
struct EquippedSkillSet
{
    static constexpr int kEquipSlots{2};
    std::array<SlotState, kEquipSlots> slots{};
    // slots[0] = key 1 (unlocks at CombatConstants::kSlot1UnlockLevel)
    // slots[1] = key 2 (unlocks at CombatConstants::kSlot2UnlockLevel)
};