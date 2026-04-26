#pragma once
/**
 * @file DataLoader.h
 * @brief Utility for loading JSON data files from the data/ directory.
 *        Paths are relative to the working directory ($(ProjectDir) in VS).
 *        All loaders call DataLoader::loadJson() rather than calling
 *        nlohmann directly, so the data root can be changed in one place.
 */

#include "nlohmann/json.hpp"
#include <string>

namespace DataLoader
{
/**
 * @brief Load and parse a JSON file at relativePath.
 * @param relativePath Path relative to the working directory (e.g. "data/items.json").
 * @throws std::runtime_error if the file cannot be opened or parsed.
 */
[[nodiscard]] nlohmann::json loadJson(const std::string &relativePath);
} // namespace DataLoader