/**
 * @file test_Inventory.cpp
 * @brief Unit tests for Inventory (add, remove, stack, gold).
 */
#include "Items/Inventory.h"
#include "Items/Item.h"
#include "doctest.h"

namespace
{
Item makeConsumable(const std::string &id)
{
    Item item{};
    item.id = id;
    item.name = id;
    item.type = ItemType::Consumable;
    item.stackSize = 5;
    return item;
}

Item makeEquipment(const std::string &id, EquipSlot slot)
{
    Item item{};
    item.id = id;
    item.name = id;
    item.type = ItemType::Equipment;
    item.equipSlot = slot;
    item.stackSize = 1;
    return item;
}
} // namespace

TEST_CASE("Inventory: addItem consumable creates stack")
{
    Inventory inv{};
    inv.addItem(makeConsumable("potion"), 3);
    CHECK(inv.getQuantity("potion") == 3);
}

TEST_CASE("Inventory: addItem consumable stacks on same id")
{
    Inventory inv{};
    inv.addItem(makeConsumable("potion"), 2);
    inv.addItem(makeConsumable("potion"), 3);
    CHECK(inv.getQuantity("potion") == 5);
}

TEST_CASE("Inventory: addItem different ids are independent")
{
    Inventory inv{};
    inv.addItem(makeConsumable("potion"), 2);
    inv.addItem(makeConsumable("ether"), 1);
    CHECK(inv.getQuantity("potion") == 2);
    CHECK(inv.getQuantity("ether") == 1);
}

TEST_CASE("Inventory: getQuantity returns 0 for absent id")
{
    Inventory inv{};
    CHECK(inv.getQuantity("nonexistent") == 0);
}

TEST_CASE("Inventory: removeItem decrements stack")
{
    Inventory inv{};
    inv.addItem(makeConsumable("potion"), 3);
    bool ok = inv.removeItem("potion", 1);
    CHECK(ok);
    CHECK(inv.getQuantity("potion") == 2);
}

TEST_CASE("Inventory: removeItem removes entry when count reaches 0")
{
    Inventory inv{};
    inv.addItem(makeConsumable("potion"), 2);
    inv.removeItem("potion", 2);
    CHECK(inv.getQuantity("potion") == 0);
    CHECK(inv.getConsumables().empty());
}

TEST_CASE("Inventory: removeItem returns false when insufficient quantity")
{
    Inventory inv{};
    inv.addItem(makeConsumable("potion"), 1);
    bool ok = inv.removeItem("potion", 5);
    CHECK(!ok);
    CHECK(inv.getQuantity("potion") == 1); // unchanged
}

TEST_CASE("Inventory: removeItem returns false for absent id")
{
    Inventory inv{};
    bool ok = inv.removeItem("ghost", 1);
    CHECK(!ok);
}

TEST_CASE("Inventory: addItem equipment is stored separately, not stacked")
{
    Inventory inv{};
    inv.addItem(makeEquipment("sword", EquipSlot::Weapon));
    inv.addItem(makeEquipment("sword", EquipSlot::Weapon)); // second copy
    CHECK(inv.getEquipment().size() == 2);
    CHECK(inv.getQuantity("sword") == 0); // not in consumable pool
}

TEST_CASE("Inventory: gold is a plain member, independently modifiable")
{
    Inventory inv{};
    inv.gold += 50;
    CHECK(inv.gold == 50);
}

TEST_CASE("Inventory: removeEquipmentAt removes the correct entry")
{
    Inventory inv{};
    Item sw{};
    sw.id = "sword";
    sw.name = "Sword";
    sw.type = ItemType::Equipment;
    sw.equipSlot = EquipSlot::Weapon;
    sw.stackSize = 1;

    Item ar{};
    ar.id = "armor";
    ar.name = "Armor";
    ar.type = ItemType::Equipment;
    ar.equipSlot = EquipSlot::Armor;
    ar.stackSize = 1;

    inv.addItem(sw);
    inv.addItem(ar);
    REQUIRE(inv.getEquipment().size() == 2);

    inv.removeEquipmentAt(0); // remove sword
    REQUIRE(inv.getEquipment().size() == 1);
    CHECK(inv.getEquipment()[0].id == "armor");
}

TEST_CASE("Inventory: removeEquipmentAt out-of-range is a no-op")
{
    Inventory inv{};
    Item sw{};
    sw.id = "sword";
    sw.type = ItemType::Equipment;
    sw.stackSize = 1;
    inv.addItem(sw);
    inv.removeEquipmentAt(5); // must not crash
    CHECK(inv.getEquipment().size() == 1);
}