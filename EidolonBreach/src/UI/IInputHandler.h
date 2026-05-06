#pragma once
/**
 * @file IInputHandler.h
 * @brief Abstract input interface for all player input 
 *
 * PlayableCharacter receives IInputHandler& in place of std::cin calls.
 */

#include <cstddef>
#include <limits>
#include <string>
#include <vector>
struct QuitException
{
};

class IInputHandler
{
  public:
    static constexpr std::size_t kCancelChoice{
        std::numeric_limits<std::size_t>::max()};

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
    /**
     * @brief Block until the player presses a number key (1–9) or Enter.
     *        Used for dungeon menus, shop, rest, etc.
     *        Up/Down arrows cycle the highlight; Enter confirms.
     * @param numOptions Number of options (valid keys: 1..numOptions).
     * @return 0-based index of chosen option.
     */
    virtual std::size_t getMenuChoice(std::size_t numOptions) = 0;

    virtual void setMenuContext(const std::string & /*title*/,
                                const std::vector<std::string> & /*options*/) {}

    /**
     * @brief Thrown by SDL3InputHandler when the user closes the window.
     *        Propagates through all blocking input calls to main().
     */
    

};
