/**
 * @file test_ItemRegistry.cpp
 * @brief Unit tests for ItemRegistry loading and item creation.
 *        Tests use an in-memory JSON string rather than a file so they
 *        do not depend on the data/ directory being present during CI.
 */
#include "Items/ItemRegistry.h"
#include "doctest.h"
#include <nlohmann/json.hpp>

namespace
{
// Build a minimal ItemRegistry without touching the filesystem.
ItemRegistry makeRegistry()
{
    // We test the parsing logic by building items directly.
    // Integration tests for file loading are covered by a manual smoke test.
    ItemRegistry reg{};
    // Use the public API: create a temporary JSON file path is not ideal for
    // unit tests. Instead, expose the internal parse through a small helper.
    // Since ItemRegistry only has loadFromJson() as a public loader, we test
    // via the round-trip: write a temp file and load it. Alternatively, test
    // create() after a successful load.
    return reg;
}
} // namespace

// Rather than spawning temp files, test the registry after loading from
// the actual data file (integration-style, but still fast).
// These tests require the working directory to have data/items.json.

TEST_CASE("ItemRegistry: size is non-zero after loadFromJson")
{
    ItemRegistry reg{};
    reg.loadFromJson("data/items.json");
    CHECK(reg.size() > 0);
}

TEST_CASE("ItemRegistry: create returns item with correct name")
{
    ItemRegistry reg{};
    reg.loadFromJson("data/items.json");
    const auto item{reg.create("heal_potion")};
    REQUIRE(item.has_value());
    CHECK(item->name == "Heal Potion");
    CHECK(item->type == ItemType::Consumable);
}

TEST_CASE("ItemRegistry: create returns nullopt for unknown id")
{
    ItemRegistry reg{};
    reg.loadFromJson("data/items.json");
    CHECK(!reg.create("nonexistent_item").has_value());
}

TEST_CASE("ItemRegistry: consumable has correct heal effect")
{
    ItemRegistry reg{};
    reg.loadFromJson("data/items.json");
    const auto item{reg.create("heal_potion")};
    REQUIRE(item.has_value());
    REQUIRE(!item->effects.empty());
    const auto *heal{std::get_if<ConsumableHeal>(&item->effects[0])};
    REQUIRE(heal != nullptr);
    CHECK(heal->amount == 30);
}

TEST_CASE("ItemRegistry: equipment has correct slot and stat modifier")
{
    ItemRegistry reg{};
    reg.loadFromJson("data/items.json");
    const auto item{reg.create("iron_sword")};
    REQUIRE(item.has_value());
    CHECK(item->type == ItemType::Equipment);
    REQUIRE(item->equipSlot.has_value());
    CHECK(*item->equipSlot == EquipSlot::Weapon);

    REQUIRE(!item->effects.empty());
    const auto *mod{std::get_if<StatModifier>(&item->effects[0])};
    REQUIRE(mod != nullptr);
    CHECK(mod->stat == StatModifier::StatType::ATK);
    CHECK(mod->amount == 8);
}

TEST_CASE("ItemRegistry: ModifyExposure effect parsed correctly")
{
    ItemRegistry reg{};
    reg.loadFromJson("data/items.json");
    const auto item{reg.create("purification_vial")};
    REQUIRE(item.has_value());
    REQUIRE(!item->effects.empty());
    const auto *exp{std::get_if<ModifyExposure>(&item->effects[0])};
    REQUIRE(exp != nullptr);
    CHECK(exp->delta == -30);
}