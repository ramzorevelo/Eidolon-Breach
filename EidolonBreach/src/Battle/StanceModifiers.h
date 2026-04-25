#pragma once
/**
 * @file StanceModifiers.h
 * @brief Free-function namespaces for Stance-based resonance modifiers and
 *        signal-to-stance ID resolution. Data-driven: add new characters by
 *        extending the if/else chains in StanceModifiers.cpp. No new classes.
 */

#include "Core/Affinity.h"
#include "Core/BehaviorSignal.h"
#include <string_view>

class PlayableCharacter;
struct BattleState;

namespace StanceModifiers
{
/**
 * @brief Resolve the dominant BehaviorSignal to a stance ID string for a character.
 *        Returns an empty string_view when the character is unrecognised.
 * @param characterId    Unit::getId() of the acting character.
 * @param dominantSignal The signal with the highest count in RunCharacterState.
 */
[[nodiscard]] std::string_view resolveStanceId(std::string_view characterId,
                                               BehaviorSignal dominantSignal);

/**
 * @brief Adjust the base Resonance Field contribution amount based on the
 *        character's crystallized Stance and the action's affinity.
 *        Returns baseAmount unchanged when stanceId is empty or unrecognised.
 *
 * @param stanceId      The crystallized stance ID from RunCharacterState.
 * @param pc            The acting character (for stat and turn context).
 * @param actionAffinity The affinity of the action being executed.
 * @param baseAmount     The floor-affinity-adjusted contribution before stance mods.
 * @param state          BattleState for turn number and field context.
 * @return               Adjusted contribution amount (always >= 0).
 */
[[nodiscard]] int applyResonanceModifier(std::string_view stanceId,
                                         const PlayableCharacter &pc,
                                         Affinity actionAffinity,
                                         int baseAmount,
                                         BattleState &state);
} // namespace StanceModifiers#pragma once
/**
 * @file StanceModifiers.h
 * @brief Free-function namespaces for Stance-based resonance modifiers and
 *        signal-to-stance ID resolution. Data-driven: add new characters by
 *        extending the if/else chains in StanceModifiers.cpp. No new classes.
 */

#include "Core/Affinity.h"
#include "Core/BehaviorSignal.h"
#include <string_view>

class PlayableCharacter;
struct BattleState;

namespace StanceModifiers
{
/**
 * @brief Resolve the dominant BehaviorSignal to a stance ID string for a character.
 *        Returns an empty string_view when the character is unrecognised.
 * @param characterId    Unit::getId() of the acting character.
 * @param dominantSignal The signal with the highest count in RunCharacterState.
 */
[[nodiscard]] std::string_view resolveStanceId(std::string_view characterId,
                                               BehaviorSignal dominantSignal);

/**
 * @brief Adjust the base Resonance Field contribution amount based on the
 *        character's crystallized Stance and the action's affinity.
 *        Returns baseAmount unchanged when stanceId is empty or unrecognised.
 *
 * @param stanceId      The crystallized stance ID from RunCharacterState.
 * @param pc            The acting character (for stat and turn context).
 * @param actionAffinity The affinity of the action being executed.
 * @param baseAmount     The floor-affinity-adjusted contribution before stance mods.
 * @param state          BattleState for turn number and field context.
 * @return               Adjusted contribution amount (always >= 0).
 */
[[nodiscard]] int applyResonanceModifier(std::string_view stanceId,
                                         const PlayableCharacter &pc,
                                         Affinity actionAffinity,
                                         int baseAmount,
                                         BattleState &state);
} // namespace StanceModifiers#pragma once
/**
 * @file StanceModifiers.h
 * @brief Free-function namespaces for Stance-based resonance modifiers and
 *        signal-to-stance ID resolution. Data-driven: add new characters by
 *        extending the if/else chains in StanceModifiers.cpp. No new classes.
 */

#include "Core/Affinity.h"
#include "Core/BehaviorSignal.h"
#include <string_view>

class PlayableCharacter;
struct BattleState;

namespace StanceModifiers
{
/**
 * @brief Resolve the dominant BehaviorSignal to a stance ID string for a character.
 *        Returns an empty string_view when the character is unrecognised.
 * @param characterId    Unit::getId() of the acting character.
 * @param dominantSignal The signal with the highest count in RunCharacterState.
 */
[[nodiscard]] std::string_view resolveStanceId(std::string_view characterId,
                                               BehaviorSignal dominantSignal);

/**
 * @brief Adjust the base Resonance Field contribution amount based on the
 *        character's crystallized Stance and the action's affinity.
 *        Returns baseAmount unchanged when stanceId is empty or unrecognised.
 *
 * @param stanceId      The crystallized stance ID from RunCharacterState.
 * @param pc            The acting character (for stat and turn context).
 * @param actionAffinity The affinity of the action being executed.
 * @param baseAmount     The floor-affinity-adjusted contribution before stance mods.
 * @param state          BattleState for turn number and field context.
 * @return               Adjusted contribution amount (always >= 0).
 */
[[nodiscard]] int applyResonanceModifier(std::string_view stanceId,
                                         const PlayableCharacter &pc,
                                         Affinity actionAffinity,
                                         int baseAmount,
                                         BattleState &state);
} // namespace StanceModifiers