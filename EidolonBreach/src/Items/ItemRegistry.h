#pragma once
/**
 * @file ItemRegistry.h
 * @brief Loads item definitions from JSON and provides lookup by ID.
 *        Owns item blueprints; callers receive copies on request.
 *        No global state — constructed and injected where needed.
 */

#include "Items/Item.h"
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include "nlohmann/json.hpp"

class ItemRegistry
{
  public:
    /**
     * @brief Load all item definitions from a JSON file.
     * @param jsonPath Path relative to working directory (e.g. "data/items.json").
     * @throws std::runtime_error if the file cannot be opened or a field is invalid.
     */
    void loadFromJson(const std::string &jsonPath);

    /**
     * @brief Create a copy of the item with the given ID.
     * @return The item, or std::nullopt if the ID is not registered.
     */
    [[nodiscard]] std::optional<Item> create(std::string_view itemId) const;

    /** @return true if itemId is registered. */
    [[nodiscard]] bool contains(std::string_view itemId) const;

    /** @return Number of registered items. */
    [[nodiscard]] std::size_t size() const;

  private:
    std::unordered_map<std::string, Item> m_items{};

    /** @brief Parse one item entry from the JSON object. */
    static Item parseItem(const std::string &id, const nlohmann::json &j);
    static ItemEffect parseEffect(const nlohmann::json &j);
};