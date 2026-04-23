/**
 * @file test_FormationManager.cpp
 * @brief Tests for FormationManager: insert, remove, swap, push, overflow.
 */
#include "Battle/FormationManager.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("FormationManager: insert places unit at correct position")
{
    FormationManager fm{};
    auto hero = makeHero("h");
    Unit *h = hero.get();
    fm.insert(h, 0);
    CHECK(fm.getAt(0) == h);
    CHECK(fm.getPosition(h) == 0);
    CHECK(fm.getCount() == 1);
}

TEST_CASE("FormationManager: insert shifts existing occupants toward rear")
{
    FormationManager fm{};
    auto h1 = makeHero("h1");
    auto h2 = makeHero("h2");
    fm.insert(h1.get(), 0);
    fm.insert(h2.get(), 0); // inserts before h1
    CHECK(fm.getAt(0) == h2.get());
    CHECK(fm.getAt(1) == h1.get());
}

TEST_CASE("FormationManager: remove closes gap and decrements count")
{
    FormationManager fm{};
    auto h1 = makeHero("h1");
    auto h2 = makeHero("h2");
    auto h3 = makeHero("h3");
    fm.insert(h1.get(), 0);
    fm.insert(h2.get(), 1);
    fm.insert(h3.get(), 2);
    fm.remove(h2.get());
    CHECK(fm.getCount() == 2);
    CHECK(fm.getAt(0) == h1.get());
    CHECK(fm.getAt(1) == h3.get()); // no gap
    CHECK(fm.getAt(2) == nullptr);
}

TEST_CASE("FormationManager: remove no-op for unit not in formation")
{
    FormationManager fm{};
    auto hero = makeHero();
    fm.remove(hero.get()); // must not crash or corrupt state
    CHECK(fm.getCount() == 0);
}

TEST_CASE("FormationManager: getPosition returns -1 for absent unit")
{
    FormationManager fm{};
    auto hero = makeHero();
    CHECK(fm.getPosition(hero.get()) == -1);
}

TEST_CASE("FormationManager: swap exchanges two occupied positions")
{
    FormationManager fm{};
    auto h1 = makeHero("h1");
    auto h2 = makeHero("h2");
    fm.insert(h1.get(), 0);
    fm.insert(h2.get(), 1);
    fm.swap(h1.get(), 1);
    CHECK(fm.getAt(0) == h2.get());
    CHECK(fm.getAt(1) == h1.get());
}

TEST_CASE("FormationManager: swap to empty slot moves unit without overflow")
{
    FormationManager fm{};
    auto hero = makeHero();
    fm.insert(hero.get(), 0);
    const bool ok{fm.swap(hero.get(), 2)};
    CHECK(ok);
    CHECK(fm.getAt(0) == nullptr);
    CHECK(fm.getAt(2) == hero.get());
}

TEST_CASE("FormationManager: applyPush moves unit within bounds; returns 0 overflow")
{
    FormationManager fm{};
    auto hero = makeHero(); // ATK 15, DEF 0
    fm.insert(hero.get(), 1);
    const Stats attacker{hero->getFinalStats()};
    const int overflow{fm.applyPush(hero.get(), 1, 1.0f, ScalingStat::ATK, attacker)};
    CHECK(overflow == 0);
    CHECK(fm.getPosition(hero.get()) == 2);
}

TEST_CASE("FormationManager: applyPush at rear boundary returns nonzero overflow")
{
    FormationManager fm{};
    auto hero = makeHero(); // ATK 15
    fm.insert(hero.get(), FormationManager::kMaxSlots - 1);
    const Stats attacker{hero->getFinalStats()};
    // Push further toward rear — clamped to last slot, overflow fires.
    const int overflow{fm.applyPush(hero.get(), 2, 1.0f, ScalingStat::ATK, attacker)};
    CHECK(overflow > 0);
    CHECK(fm.getPosition(hero.get()) == FormationManager::kMaxSlots - 1);
}

TEST_CASE("FormationManager: applyPush at front boundary returns nonzero overflow")
{
    FormationManager fm{};
    auto hero = makeHero();
    fm.insert(hero.get(), 0);
    const Stats attacker{hero->getFinalStats()};
    const int overflow{fm.applyPush(hero.get(), -1, 1.0f, ScalingStat::ATK, attacker)};
    CHECK(overflow > 0);
    CHECK(fm.getPosition(hero.get()) == 0);
}

TEST_CASE("FormationManager: insert returns false when formation is full")
{
    FormationManager fm{};
    std::vector<std::unique_ptr<PlayableCharacter>> heroes{};
    for (int i{0}; i < FormationManager::kMaxSlots; ++i)
    {
        heroes.push_back(makeHero("h" + std::to_string(i)));
        fm.insert(heroes.back().get(), i);
    }
    auto extra = makeHero("extra");
    CHECK(!fm.insert(extra.get(), 0));
    CHECK(fm.getCount() == FormationManager::kMaxSlots);
}