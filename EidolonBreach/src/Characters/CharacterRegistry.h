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
class MetaProgress;

class CharacterRegistry
{
  public:
    void loadFromJson(const std::string &jsonPath,
                      const AbilityRegistry &abilityRegistry);

    /**
     * @brief Construct a fully equipped PlayableCharacter.
     * @param characterId  Registry key (e.g. "lyra").
     * @param characterLevel Level used for slot/arch-skill unlocks.
     * @param meta         If non-null, activated Aspect Tree nodes and Echo
     *                     effects from this MetaProgress are applied.
     */
    [[nodiscard]] std::unique_ptr<PlayableCharacter>
    create(std::string_view characterId,
           int characterLevel = 1,
           const MetaProgress *meta = nullptr) const;

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
        std::vector<std::string> slotSkillIds{};
        std::string archetype{};
        std::string basicId{};
        std::string archSkillId{};
        std::string ultimateId{};
        float fractureSelfDotPct{0.0f};
        float breachbornActionBonusDivisor{0.0f};
        int breachbornActionBurnDamage{0};
        int breachbornActionBurnDuration{0};
    };

    std::vector<std::string> m_orderedIds{};
    std::unordered_map<std::string, CharacterBlueprint> m_blueprints{};
    const AbilityRegistry *m_abilityRegistry{nullptr};

    static CharacterBlueprint parseBlueprint(const std::string &id,
                                             const nlohmann::json &j);
    static Affinity parseAffinity(const std::string &s);
    /**
     * @brief Apply all activated Aspect Tree node effects from meta to pc.
     */
    static void applyProgressionMods(std::string_view characterId,
                                     const MetaProgress &meta,
                                     PlayableCharacter &pc);

    /**
     * @brief Apply Echo 1–5 effects for the given character.
     *        No-op if echoCount is 0. Each Echo stacks independently.
     */
    static void applyEchoEffects(std::string_view characterId,
                                 int echoCount,
                                 PlayableCharacter &pc);
};