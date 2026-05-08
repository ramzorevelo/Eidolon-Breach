#pragma once
/**
 * @file SDL3InputHandler.h
 * @brief SDL3 keyboard and mouse implementation of IInputHandler.
 *        Blocks on SDL_WaitEvent until valid input arrives.
 *        Keyboard: Q/E/1/2/R/V for actions; Up/Down for targets; Enter confirms.
 *        Mouse: left-click on action rows or unit cards; right-click cancels;
 *        scroll wheel controls log offset.
 */

#include "UI/IInputHandler.h"
#include "UI/SDL3Renderer.h"
#include <cstddef>

class SDL3InputHandler : public IInputHandler
{
  public:
    explicit SDL3InputHandler(SDL3Renderer &renderer) : m_renderer{renderer} {}

    /**
     * @brief Block until the player presses a valid action key.
     *        Keys: Q=0, E=1, 1=2, 2=3, R=4, V=5.
     *        Only indices in [0, numActions) are accepted.
     * @param numActions Number of available actions.
     * @return 0-based action index.
     */
    std::size_t getActionChoice(std::size_t numActions) override;

    /**
     * @brief Block until the player selects a target.
     *        Left/Right arrows cycle; Enter confirms the highlighted index.
     * @param numTargets Number of valid targets.
     * @return 0-based target index.
     */
    std::size_t getTargetChoice(std::size_t numTargets) override;
    std::size_t getMenuChoice(std::size_t numOptions) override;

    void setMenuContext(const std::string &title,
                        const std::vector<std::string> &options)
    {
        m_menuTitle = title;
        m_menuOptions = options;
    }
  private:
    SDL3Renderer &m_renderer;
    std::size_t m_bufferedActionIdx{std::numeric_limits<std::size_t>::max()};
    int m_pendingTargetIdx{-1};
    std::string m_menuTitle{};
    std::vector<std::string> m_menuOptions{};
};