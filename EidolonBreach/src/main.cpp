/**
 * @file main.cpp
 * @brief Entry point. Loads registries from data/ and runs a dungeon.
 */
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Characters/AbilityRegistry.h"
#include "Characters/CharacterRegistry.h"
#include "Characters/Lyra/EmberCallAction.h"
#include "Characters/Lyra/Ignis.h"
#include "Characters/Lyra/LyraUltimateAction.h"
#include "Characters/Vex/VexBulwarkAction.h"
#include "Characters/Vex/VexUltimateAction.h"
#include "Characters/Zara/ZaraFrostbindAction.h"
#include "Characters/Zara/ZaraUltimateAction.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Summons/SummonRegistry.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <memory>
#include <random>
#include "Core/Settings.h"
#include "UI/HubScreen.h"

static AbilityRegistry buildAbilityRegistry()
{
    AbilityRegistry reg{};
    reg.registerAbility("basic_strike",
                        []
                        { return std::make_unique<BasicStrikeAction>(); });
    reg.registerAbility("arch_skill_default",
                        []
                        { return std::make_unique<SkillAction>(2.0f); });
    reg.registerAbility("lyra_ultimate",
                        []
                        { return std::make_unique<LyraUltimateAction>(); });
    reg.registerAbility("vex_ultimate",
                        []
                        { return std::make_unique<VexUltimateAction>(); });
    reg.registerAbility("zara_ultimate",
                        []
                        { return std::make_unique<ZaraUltimateAction>(); });
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


int main()
{
    // Registries
    AbilityRegistry abilityRegistry{buildAbilityRegistry()};

    CharacterRegistry characterRegistry{};
    characterRegistry.loadFromJson("data/characters.json", abilityRegistry);

    SummonRegistry summonRegistry{};
    Ignis::registerIgnis(summonRegistry);

    SDL3Renderer renderer{"Eidolon Breach", 1280, 720};
    SDL3InputHandler input{renderer, renderer};

    // MetaProgress
    MetaProgress meta{MetaProgress::loadFromFile("save.json")};
    Settings settings{Settings::loadFromFile("settings.json")};

    renderer.setResolution(settings.resolutionWidth, settings.resolutionHeight);

    // Unlock starting characters if this is a fresh save.
    for (const std::string &id : characterRegistry.getIds())
        std::ignore = meta.unlockCharacter(id);

    try
    {
        HubScreen hub{renderer, input, characterRegistry, abilityRegistry,
                      summonRegistry, meta, settings};
        hub.run();
    }
    catch (const QuitException &)
    {
        renderer.clearBattleCache();
    }

    meta.saveToFile("save.json");

    return 0;
}