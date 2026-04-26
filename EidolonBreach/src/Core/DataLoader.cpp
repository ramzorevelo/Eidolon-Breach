/**
 * @file DataLoader.cpp
 * @brief DataLoader implementation.
 */

#include "Core/DataLoader.h"
#include <fstream>
#include <stdexcept>

namespace DataLoader
{
nlohmann::json loadJson(const std::string &relativePath)
{
    std::ifstream file{relativePath};
    if (!file.is_open())
        throw std::runtime_error{"DataLoader: cannot open '" + relativePath + "'"};

    nlohmann::json j{};
    try
    {
        file >> j;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        throw std::runtime_error{"DataLoader: JSON parse error in '" +
                                 relativePath + "': " + e.what()};
    }
    return j;
}
} // namespace DataLoader