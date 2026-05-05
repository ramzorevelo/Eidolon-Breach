
/**
 * @file test_CharacterRegistry.cpp
 * @brief Tests for CharacterRegistry and AbilityRegistry.
 */
#include "Characters/Lyra/LyraUltimateAction.h"
#include "Characters/Vex/VexUltimateAction.h"
#include "Characters/Zara/ZaraUltimateAction.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Characters/Lyra/EmberCallAction.h"
#include "Characters/Vex/VexBulwarkAction.h"
#include "Characters/Zara/ZaraFrostbindAction.h"
#include "Entities/PlayableCharacter.h"
#include "Characters/AbilityRegistry.h"
#include "Characters/CharacterRegistry.h"
#include "doctest.h"

namespace
{
AbilityRegistry makeAbilityRegistry()
{
    AbilityRegistry reg{};
    reg.registerAbility("basic_strike",
                        []
                        { return std::make_unique<BasicStrikeAction>(); });
    reg.registerAbility("arch_skill_default",
                        []
                        { return std::make_unique<SkillAction>(2.0f); });
    reg.registerAbility("ultimate_default",
                        []
                        { return std::make_unique<UltimateAction>(); });
    reg.registerAbility("lyra_ultimate",
                        []
                        { return std::make_unique<LyraUltimateAction>(); });
    reg.registerAbility("vex_ultimate",
                        []
                        { return std::make_unique<VexUltimateAction>(); });
    reg.registerAbility("zara_ultimate",
                        []
                        { return std::make_unique<ZaraUltimateAction>(); });
    // Slot skills (required for pre‑unlock tests)
    reg.registerAbility("lyra_ember_call",
                        []
                        { return std::make_unique<EmberCallAction>(); });
    reg.registerAbility("vex_bulwark",
                        []
                        { return std::make_unique<VexBulwarkAction>(); });
    reg.registerAbility("zara_frostbind",
                        []
                        { return std::make_unique<ZaraFrostbindAction>(); });
    return reg;
}
} // namespace

TEST_CASE("AbilityRegistry: create returns nullptr for unknown id")
{
    AbilityRegistry reg{makeAbilityRegistry()};
    CHECK(reg.create("nonexistent") == nullptr);
}

TEST_CASE("AbilityRegistry: create returns unique instances each call")
{
    AbilityRegistry reg{makeAbilityRegistry()};
    auto a{reg.create("basic_strike")};
    auto b{reg.create("basic_strike")};
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(a.get() != b.get()); // distinct allocations
}

TEST_CASE("CharacterRegistry: create returns nullptr for unknown id")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);
    CHECK(chars.create("nobody") == nullptr);
}

TEST_CASE("CharacterRegistry: create returns PlayableCharacter with correct stats")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    auto pc{chars.create("lyra")};
    REQUIRE(pc != nullptr);
    CHECK(pc->getName() == "Lyra");
    CHECK(pc->getBaseStats().maxHp == 80);
    CHECK(pc->getBaseStats().atk == 22);
    CHECK(pc->getBaseStats().spd == 14);
    CHECK(pc->getResonanceContribution() == 10);
}

TEST_CASE("CharacterRegistry: created character has all three abilities registered")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    auto pc{chars.create("vex")};
    REQUIRE(pc != nullptr);
    // basic + archSkill + ultimate + slot1 + vent = 5
    CHECK(pc->getAbilities().size() == 5);
}

TEST_CASE("CharacterRegistry: getIds returns all registered ids in order")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);
    const auto &ids{chars.getIds()};
    REQUIRE(ids.size() == 3);
    CHECK(ids[0] == "lyra");
    CHECK(ids[1] == "vex");
    CHECK(ids[2] == "zara");
}
TEST_CASE("CharacterRegistry: getArchetype returns correct archetype string")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    CHECK(chars.getArchetype("lyra") == "Striker");
    CHECK(chars.getArchetype("vex") == "Conduit");
    CHECK(chars.getArchetype("nobody") == "");
}

TEST_CASE("CharacterRegistry: created character has archetype set correctly")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    auto lyra{chars.create("lyra")};
    REQUIRE(lyra != nullptr);
    CHECK(lyra->getArchetype() == "Striker");

    auto vex{chars.create("vex")};
    REQUIRE(vex != nullptr);
    CHECK(vex->getArchetype() == "Conduit");

    auto zara{chars.create("zara")};
    REQUIRE(zara != nullptr);
    CHECK(zara->getArchetype() == "Weaver");
}

TEST_CASE("CharacterRegistry: create Zara has correct stats and archetype")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    auto pc{chars.create("zara")};
    REQUIRE(pc != nullptr);
    CHECK(pc->getName() == "Zara");
    CHECK(pc->getBaseStats().maxHp == 90);
    CHECK(pc->getBaseStats().spd == 12);
    CHECK(pc->getBaseStats().atk == 14);
    CHECK(chars.getArchetype("zara") == "Weaver");
}

TEST_CASE("CharacterRegistry: Lyra has Slot 1 pre-unlocked and equipped")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    auto pc{chars.create("lyra")};
    REQUIRE(pc != nullptr);
    const auto &slots{pc->getEquippedSkills()};
    CHECK(slots.slots[0].unlocked);
    CHECK(slots.slots[0].equippedSkill != nullptr);
    CHECK(!slots.slots[1].unlocked);
}

TEST_CASE("CharacterRegistry: Vex Slot 1 pre-unlocked with EarthenShield")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    auto pc{chars.create("vex")};
    REQUIRE(pc != nullptr);
    CHECK(pc->getEquippedSkills().slots[0].unlocked);
    CHECK(pc->getEquippedSkills().slots[0].equippedSkill != nullptr);
    CHECK(pc->getEquippedSkills().slots[0].equippedSkill->label() ==
          "Earthen Shield (25 SP - Shield one ally)");
}


TEST_CASE("CharacterRegistry: all characters have VentAction in ability list")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);

    for (const auto &id : chars.getIds())
    {
        auto pc{chars.create(id)};
        REQUIRE(pc != nullptr);
        bool hasVent{false};
        for (const auto &ability : pc->getAbilities())
        {
            if (ability->getActionData().category == ActionCategory::Vent)
            {
                hasVent = true;
                break;
            }
        }
        CHECK(hasVent); // every Synchron must have Vent
    }
}

