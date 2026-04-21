/**
 * @file test_UseConsumableAction.cpp
 * @brief Tests for UseConsumableAction: availability, heal, exposure, cooldown,
 *        and inventory quantity decrement.
 */
#include "Actions/UseConsumableAction.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Items/Item.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

namespace
{
Item makeHealPotion(int amount)
{
    Item item{};
    item.id = "heal_potion";
    item.name = "Heal Potion";
    item.type = ItemType::Consumable;
    item.effects.push_back(ConsumableHeal{amount});
    return item;
}

Item makePurificationVial(int exposureDelta)
{
    Item item{};
    item.id = "purif_vial";
    item.name = "Purification Vial";
    item.type = ItemType::Consumable;
    item.effects.push_back(ModifyExposure{exposureDelta});
    return item;
}
} // namespace

TEST_CASE("UseConsumableAction: isAvailable true when cooldown 0 and item in inventory")
{
    Party allies;
    allies.getInventory().addItem(makeHealPotion(30), 1);
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makeHealPotion(30)};
    CHECK(action.isAvailable(*heroPtr, allies));
}

TEST_CASE("UseConsumableAction: isAvailable false when item not in inventory")
{
    Party allies;
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makeHealPotion(30)};
    CHECK(!action.isAvailable(*heroPtr, allies));
}

TEST_CASE("UseConsumableAction: isAvailable false when consumable on cooldown")
{
    Party allies;
    allies.getInventory().addItem(makeHealPotion(30), 2);
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    heroPtr->consumeConsumableAction(false); // put on cooldown
    UseConsumableAction action{makeHealPotion(30)};
    CHECK(!action.isAvailable(*heroPtr, allies));
}

TEST_CASE("UseConsumableAction: heal consumable restores HP")
{
    Party allies, enemies;
    allies.getInventory().addItem(makeHealPotion(30), 1);
    auto heroRaw = makeHero(); // 120/120 HP
    auto *heroPtr = heroRaw.get();
    heroPtr->takeTrueDamage(50); // 70 HP
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makeHealPotion(30)};
    ActionResult result = action.execute(*heroPtr, allies, enemies, std::nullopt);
    CHECK(result.type == ActionResult::Type::Heal);
    CHECK(result.value == 30);
    CHECK(heroPtr->getHp() == 100);
}

TEST_CASE("UseConsumableAction: heal is capped at maxHp")
{
    Party allies, enemies;
    allies.getInventory().addItem(makeHealPotion(100), 1);
    auto heroRaw = makeHero(); // already at 120/120
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makeHealPotion(100)};
    action.execute(*heroPtr, allies, enemies, std::nullopt);
    CHECK(heroPtr->getHp() == 120); // capped
}

TEST_CASE("UseConsumableAction: exposure consumable reduces exposure")
{
    Party allies, enemies;
    allies.getInventory().addItem(makePurificationVial(-30), 1);
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    heroPtr->modifyExposure(60);
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makePurificationVial(-30)};
    ActionResult result = action.execute(*heroPtr, allies, enemies, std::nullopt);
    CHECK(heroPtr->getExposure() == 30);
    CHECK(result.exposureDelta == -30);
}

TEST_CASE("UseConsumableAction: decrements inventory quantity on execute")
{
    Party allies, enemies;
    allies.getInventory().addItem(makeHealPotion(30), 3);
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makeHealPotion(30)};
    action.execute(*heroPtr, allies, enemies, std::nullopt);
    CHECK(allies.getInventory().getQuantity("heal_potion") == 2);
}

TEST_CASE("UseConsumableAction: sets consumable cooldown after use")
{
    Party allies, enemies;
    allies.getInventory().addItem(makeHealPotion(30), 2);
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    UseConsumableAction action{makeHealPotion(30)};
    action.execute(*heroPtr, allies, enemies, std::nullopt);
    CHECK(!heroPtr->canUseConsumable()); // cooldown is now active
}

TEST_CASE("UseConsumableAction: label contains item name")
{
    UseConsumableAction action{makeHealPotion(30)};
    CHECK(action.label() == "Use: Heal Potion");
}

TEST_CASE("UseConsumableAction: category is Consumable")
{
    UseConsumableAction action{makeHealPotion(30)};
    CHECK(action.getActionData().category == ActionCategory::Consumable);
}