/**
 * @file test_AspectTree.cpp
 * @brief Tests for AspectTree loading, node lookup, and CharacterMod parsing.
 */
#include "Core/CharacterMod.h"
#include "Meta/AspectTree.h"
#include "doctest.h"
#include <variant>

TEST_CASE("AspectTree: loadForCharacter returns empty tree for unknown id")
{
    const AspectTree tree{AspectTree::loadForCharacter("nonexistent_character")};
    CHECK(tree.nodes().empty());
}

TEST_CASE("AspectTree: loadForCharacter loads lyra tree with 15 nodes")
{
    const AspectTree tree{AspectTree::loadForCharacter("lyra")};
    CHECK(tree.nodes().size() == 15);
}

TEST_CASE("AspectTree: findNode returns nullptr for unknown id")
{
    const AspectTree tree{AspectTree::loadForCharacter("lyra")};
    CHECK(tree.findNode("nonexistent_node") == nullptr);
}

TEST_CASE("AspectTree: findNode returns correct node")
{
    const AspectTree tree{AspectTree::loadForCharacter("lyra")};
    const AspectTreeNode *node{tree.findNode("lyra_aggressive_1")};
    REQUIRE(node != nullptr);
    CHECK(node->branch == BehaviorSignal::Aggressive);
    CHECK(node->unlockThreshold == 5);
    CHECK(node->insightCost == 3);
    CHECK(std::holds_alternative<StatBonus>(node->effect));
    CHECK(std::get<StatBonus>(node->effect).atk == 2);
}

TEST_CASE("AspectTree: CooldownReduction node parsed correctly")
{
    const AspectTree tree{AspectTree::loadForCharacter("lyra")};
    const AspectTreeNode *node{tree.findNode("lyra_methodical_1")};
    REQUIRE(node != nullptr);
    CHECK(std::holds_alternative<CooldownReduction>(node->effect));
    CHECK(std::get<CooldownReduction>(node->effect).turns == 1);
}

TEST_CASE("AspectTree: AVModifierBonus node parsed correctly")
{
    const AspectTree tree{AspectTree::loadForCharacter("lyra")};
    const AspectTreeNode *node{tree.findNode("lyra_sacrificial_1")};
    REQUIRE(node != nullptr);
    REQUIRE(std::holds_alternative<AVModifierBonus>(node->effect));
    const auto &bonus{std::get<AVModifierBonus>(node->effect)};
    CHECK(bonus.state == ExposureState::Fractured);
    CHECK(bonus.delta == doctest::Approx(-0.05f));
}