/**
 * @file ConsoleInputHandler.cpp
 * @brief ConsoleInputHandler implementation.
 */

#include "UI/ConsoleInputHandler.h"
#include <iostream>
#include <limits>

namespace
{
std::size_t readChoice(std::size_t maxIndex)
{
    while (true)
    {
        int choice{};
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice >= 1 && static_cast<std::size_t>(choice) <= maxIndex)
            return static_cast<std::size_t>(choice - 1);

        std::cout << "Invalid choice.\n";
    }
}
} // namespace

std::size_t ConsoleInputHandler::getActionChoice(std::size_t numActions)
{
    std::cout << "Choose action: ";
    return readChoice(numActions);
}

std::size_t ConsoleInputHandler::getTargetChoice(std::size_t numTargets)
{
    std::cout << "Target: ";
    return readChoice(numTargets);
}