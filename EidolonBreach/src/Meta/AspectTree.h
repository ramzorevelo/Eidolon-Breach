#pragma once
/**
 * @file AspectTree.h
 * @brief Aspect Tree data structure for one character.
 *        Loaded from data/aspect_trees/<character_id>.json.
 *        Nodes are grouped into five branches keyed by BehaviorSignal.
 */
#include "Core/BehaviorSignal.h"
#include "Core/CharacterMod.h"
#include <string>
#include <vector>

/** One node in an Aspect Tree branch. */
struct AspectTreeNode
{
    std::string id{};
    BehaviorSignal branch{BehaviorSignal::Aggressive};
    int unlockThreshold{0}; 
    int insightCost{0};     
    CharacterMod effect{StatBonus{}};
};

class AspectTree
{
  public:
    /**
     * @brief Load the tree for characterId from data/aspect_trees/<id>.json.
     *        Returns an empty tree if the file does not exist or parse fails.
     * @param characterId Character's string ID (e.g. "lyra").
     */
    [[nodiscard]] static AspectTree loadForCharacter(std::string_view characterId);

    /** @return All nodes in definition order. */
    [[nodiscard]] const std::vector<AspectTreeNode> &nodes() const;

    /**
     * @brief Find a node by ID.
     * @return nullptr if no node with that ID exists.
     */
    [[nodiscard]] const AspectTreeNode *findNode(std::string_view nodeId) const;

  private:
    std::vector<AspectTreeNode> m_nodes{};
};