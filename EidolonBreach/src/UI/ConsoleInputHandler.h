#pragma once
/**
 * @file ConsoleInputHandler.h
 * @brief std::cin implementation of IInputHandler 
 */

#include "UI/IInputHandler.h"

/** Reads player choices from std::cin with validation. */
class ConsoleInputHandler : public IInputHandler
{
  public:
    std::size_t getActionChoice(std::size_t numActions) override;
    std::size_t getTargetChoice(std::size_t numTargets) override;
};