/**
 * @file test_Equipment.cpp
 * @brief Tests for PlayableCharacter equipment slots, stat pre-pass, and
 *        ResonanceModifier at equip/unequip time.
 */
#include "Entities/PlayableCharacter.h"
#include "Items/Item.h"
#include "doctest.h"
#include "test_helpers.h"

namespace
{
Item makeWeapon(int atkBonus)
{
    Item w{};
    w.id = "weapon_test";
    w.name = "Test Sword";
    w.type = ItemType::Equipment;
    w.equipSlot = EquipSlot::Weapon;
    w.effects.push_back(StatModifier{StatModifier::StatType::ATK, atkBonus});
    return w;
}

Item makeArmor(int defBonus)
{
    Item a{};
    a.id = "armor_test";
    a.name = "Test Armor";
    a.type = ItemType::Equipment;
    a.equipSlot = EquipSlot::Armor;
    a.effects.push_back(StatModifier{StatModifier::StatType::DEF, defBonus});
    return a;
}

Item makeAccessory(int resonanceBonus)
{
    Item acc{};
    acc.id = "acc_test";
    acc.name = "Test Accessory";
    acc.type = ItemType::Equipment;
    acc.equipSlot = EquipSlot::Accessory;
    acc.effects.push_back(ResonanceModifier{resonanceBonus});
    return acc;
}
} // namespace

TEST_CASE("Equipment: getFinalStats reflects weapon ATK bonus")
{
    auto hero = makeHero(); // base ATK = 15
    hero->equip(makeWeapon(10));
    CHECK(hero->getFinalStats().atk == 25);
    CHECK(hero->getBaseStats().atk == 15); // base unchanged
}

TEST_CASE("Equipment: getFinalStats reflects armor DEF bonus")
{
    auto hero = makeHero(); // base DEF = 0
    hero->equip(makeArmor(8));
    CHECK(hero->getFinalStats().def == 8);
}

TEST_CASE("Equipment: unequip removes stat bonus")
{
    auto hero = makeHero();
    hero->equip(makeWeapon(10));
    REQUIRE(hero->getFinalStats().atk == 25);
    hero->unequip(EquipSlot::Weapon);
    CHECK(hero->getFinalStats().atk == 15);
}

TEST_CASE("Equipment: equip returns displaced item when slot was occupied")
{
    auto hero = makeHero();
    hero->equip(makeWeapon(5));
    auto displaced = hero->equip(makeWeapon(10));
    REQUIRE(displaced.has_value());
    CHECK(displaced->id == "weapon_test");
    CHECK(hero->getFinalStats().atk == 25); // new weapon applied
}

TEST_CASE("Equipment: unequip returns nullopt when slot is empty")
{
    auto hero = makeHero();
    auto result = hero->unequip(EquipSlot::Weapon);
    CHECK(!result.has_value());
}

TEST_CASE("Equipment: ResonanceModifier applied immediately at equip")
{
    auto hero = makeHero(); // resonanceContribution = 10
    hero->equip(makeAccessory(5));
    CHECK(hero->getResonanceContribution() == 15);
}

TEST_CASE("Equipment: ResonanceModifier reversed on unequip")
{
    auto hero = makeHero();
    hero->equip(makeAccessory(5));
    hero->unequip(EquipSlot::Accessory);
    CHECK(hero->getResonanceContribution() == 10);
}

TEST_CASE("Equipment: multiple slots apply independently in stat pre-pass")
{
    auto hero = makeHero(); // ATK 15, DEF 0
    hero->equip(makeWeapon(10));
    hero->equip(makeArmor(5));
    CHECK(hero->getFinalStats().atk == 25);
    CHECK(hero->getFinalStats().def == 5);
}

TEST_CASE("Equipment: non-Equipment item passed to equip() returns nullopt (no-op)")
{
    auto hero = makeHero();
    Item consumable{};
    consumable.id = "potion";
    consumable.type = ItemType::Consumable;
    auto result = hero->equip(consumable);
    CHECK(!result.has_value());
    CHECK(!hero->getEquipment().weapon.has_value());
}