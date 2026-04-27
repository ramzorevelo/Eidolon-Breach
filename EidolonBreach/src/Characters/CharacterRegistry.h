#pragma once
/**
 * @file CharacterRegistry.h
 * @brief Loads character definitions from JSON and creates fully equipped
 *        PlayableCharacter instances. Abilities are resolved via AbilityRegistry.
 */

#include "Core/Affinity.h"
#include "nlohmann/json.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class PlayableCharacter;
class AbilityRegistry;

class CharacterRegistry
{
  public:
    void loadFromJson(const std::string &jsonPath,
                      const AbilityRegistry &abilityRegistry);

    [[nodiscard]] std::unique_ptr<PlayableCharacter>
    create(std::string_view characterId) const;

    [[nodiscard]] const std::vector<std::string> &getIds() const;
    [[nodiscard]] bool contains(std::string_view characterId) const;
    /**
     * @brief Returns the archetype string for characterId, or empty string if not found.
     *        Useful for UI display without creating a PlayableCharacter instance.
     */
    [[nodiscard]] std::string getArchetype(std::string_view characterId) const;
  private:
    struct CharacterBlueprint
    {
        std::string id{};
        std::string name{};
        std::string affinity{};
        int maxHp{};
        int atk{};
        int def{};
        int spd{};
        int resonanceContribution{};
        std::string passiveTrait{};
        std::string archetype{};
        std::string basicId{};
        std::string archSkillId{};
        std::string ultimateId{};
    };

    std::vector<std::string> m_orderedIds{};
    std::unordered_map<std::string, CharacterBlueprint> m_blueprints{};
    const AbilityRegistry *m_abilityRegistry{nullptr};

    static CharacterBlueprint parseBlueprint(const std::string &id,
                                             const nlohmann::json &j);
    static Affinity parseAffinity(const std::string &s);
};