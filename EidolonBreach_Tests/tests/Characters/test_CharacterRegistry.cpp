/**
 * @file test_CharacterRegistry.cpp
 * @brief Tests for CharacterRegistry and AbilityRegistry.
 */
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
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

    auto pc{chars.create("striker_1")};
    REQUIRE(pc != nullptr);
    CHECK(pc->getName() == "Striker");
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

    auto pc{chars.create("conduit_1")};
    REQUIRE(pc != nullptr);
    // basic + archSkill + ultimate = 3
    CHECK(pc->getAbilities().size() == 3);
}

TEST_CASE("CharacterRegistry: getIds returns all registered ids in order")
{
    AbilityRegistry abilities{makeAbilityRegistry()};
    CharacterRegistry chars{};
    chars.loadFromJson("data/characters.json", abilities);
    const auto &ids{chars.getIds()};
    REQUIRE(ids.size() == 2);
    CHECK(ids[0] == "striker_1");
    CHECK(ids[1] == "conduit_1");
}