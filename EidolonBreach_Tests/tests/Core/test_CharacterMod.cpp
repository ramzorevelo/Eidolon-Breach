/**
 * @file test_CharacterMod.cpp
 * @brief Tests for PlayableCharacter::applyCharacterMod — all six mod types.
 */
#include "Core/CharacterMod.h"
#include "Core/CombatConstants.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("applyCharacterMod: StatBonus increases maxHp and resets current HP")
{
    auto hero{makeHero()};
    const int oldMax{hero->getBaseStats().maxHp};
    hero->applyCharacterMod(StatBonus{10, 0, 0, 0});
    CHECK(hero->getBaseStats().maxHp == oldMax + 10);
    CHECK(hero->getHp() == hero->getBaseStats().maxHp);
}

TEST_CASE("applyCharacterMod: StatBonus increases ATK")
{
    auto hero{makeHero()};
    const int oldAtk{hero->getBaseStats().atk};
    hero->applyCharacterMod(StatBonus{0, 3, 0, 0});
    CHECK(hero->getBaseStats().atk == oldAtk + 3);
}

TEST_CASE("applyCharacterMod: CooldownReduction lowers effectiveArchSkillCooldown")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(CooldownReduction{1});
    CHECK(hero->effectiveArchSkillCooldown() ==
          CombatConstants::kArchSkillCooldownTurns - 1);
}

TEST_CASE("applyCharacterMod: CooldownReduction clamps effective cooldown to 0")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(CooldownReduction{10});
    CHECK(hero->effectiveArchSkillCooldown() == 0);
}

TEST_CASE("applyCharacterMod: ExposureThresholdShift lowers Resonating threshold")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(ExposureThresholdShift{50, -5});
    // Verify via getExposureAVModifier at exposure 45 (normally below Resonating).
    hero->modifyExposure(45);
    const float mod{hero->getExposureAVModifier()};
    CHECK(mod == doctest::Approx(CombatConstants::kAvModResonating));
}

TEST_CASE("applyCharacterMod: ProcEnhancement multiplies procEnhancementMultiplier")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(ProcEnhancement{1.20f});
    CHECK(hero->procEnhancementMultiplier() == doctest::Approx(1.20f));
}

TEST_CASE("applyCharacterMod: multiple ProcEnhancement mods multiply together")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(ProcEnhancement{1.10f});
    hero->applyCharacterMod(ProcEnhancement{1.10f});
    CHECK(hero->procEnhancementMultiplier() == doctest::Approx(1.21f));
}

TEST_CASE("applyCharacterMod: AVModifierBonus adjusts Fractured modifier")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(AVModifierBonus{ExposureState::Fractured, -0.05f});
    hero->applyFracture();
    CHECK(hero->getExposureAVModifier() ==
          doctest::Approx(CombatConstants::kAvModFractured - 0.05f));
}

TEST_CASE("applyCharacterMod: SlotUnlockEarly lowers slot 0 unlock level")
{
    auto hero{makeHero()};
    hero->applyCharacterMod(SlotUnlockEarly{0, 10});
    hero->applyUnlocks(10);
    CHECK(hero->getEquippedSkills().slots[0].unlocked);
}