#pragma once
/**
 * @file test_NullInputHandler.h
 * @brief No-op IInputHandler for use in unit tests.
 */

#include "UI/IInputHandler.h"

class NullInputHandler : public IInputHandler
{
  public:
    std::size_t getActionChoice(std::size_t) override
    {
        return 0;
    }
    std::size_t getTargetChoice(std::size_t) override
    {
        return 0;
    }
};