#pragma once
/**
 * @file IInputHandler.h
 * @brief Abstract input interface for all player input 
 *
 * PlayableCharacter receives IInputHandler& in place of std::cin calls.
 */

#include <cstddef>

class IInputHandler
{
  public:
    virtual ~IInputHandler() = default;

    /**
     * @brief Get the player's action selection.
     * @param numActions Number of available actions (valid choices: 1..numActions).
     * @return 0-based index of the chosen action.
     */
    virtual std::size_t getActionChoice(std::size_t numActions) = 0;

    /**
     * @brief Get the player's target selection.
     * @param numTargets Number of valid targets (valid choices: 1..numTargets).
     * @return 0-based index of the chosen target within the alive-target list.
     */
    virtual std::size_t getTargetChoice(std::size_t numTargets) = 0;
};
