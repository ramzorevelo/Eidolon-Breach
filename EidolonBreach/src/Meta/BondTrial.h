#pragma once
/**
 * @file BondTrial.h
 * @brief Runs a character-specific Bond Trial combat challenge from the hub.
 *        Completion sets `bondTrialComplete` in MetaProgress and awards Insight.
 */
#include <string_view>

class MetaProgress;
class CharacterRegistry;
class AbilityRegistry;
class SummonRegistry;
class SDL3Renderer;
class SDL3InputHandler;

namespace BondTrial
{

/**
 * @brief Run the Bond Trial for `characterId`.
 *        Uses a hardcoded enemy encounter specific to each character.
 *        Awards kBondTrialInsightBonus Insight on victory.
 *        No-op (shows message) if the trial is already complete.
 * @return true if the player won.
 */
bool run(std::string_view characterId,
         MetaProgress &meta,
         const CharacterRegistry &charRegistry,
         const AbilityRegistry &abilityRegistry,
         const SummonRegistry &summonRegistry,
         SDL3Renderer &renderer,
         SDL3InputHandler &input);

} // namespace BondTrial