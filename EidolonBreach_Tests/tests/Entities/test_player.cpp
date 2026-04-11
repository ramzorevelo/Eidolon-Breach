#include "doctest.h"
#include "Entities/Player.h"

TEST_CASE("Player initialises with correct defaults") {
    Player p{ "Rina" };

    CHECK(p.getHp() == 120);
    CHECK(p.getSp() == 3);
    CHECK(p.getEnergy() == 0);
    CHECK(p.ultimateReady() == false);
}
TEST_CASE("Player::basicAttack") {
    Player p{ "Rina" };
    ActionResult r = p.basicAttack();

    SUBCASE("deals 15 damage") {
        CHECK(r.value == 15);
    }
    SUBCASE("returns damage type") {
        CHECK(r.type == ActionResult::Type::Damage);
    }
    SUBCASE("grants one SP") {
        CHECK(p.getSp() == 4);
    }
    SUBCASE("grants 20 energy") {
        CHECK(p.getEnergy() == 20);
    }
}

TEST_CASE("Player::useSkill") {
    Player p{ "Rina" };
    ActionResult r = p.useSkill();

    SUBCASE("deals 28 damage") {
        CHECK(r.value == 28);
    }
    SUBCASE("returns damage type") {
        CHECK(r.type == ActionResult::Type::Damage);
    }
    SUBCASE("costs one SP") {
        CHECK(p.getSp() == 2);
    }
    SUBCASE("grants 30 energy") {
        CHECK(p.getEnergy() == 30);
    }
}

TEST_CASE("Player::useUltimate") {
    Player p{ "Rina" };
    for (int i{ 0 }; i < 5; ++i) p.basicAttack();
    REQUIRE(p.ultimateReady() == true);
    ActionResult r = p.useUltimate();

    SUBCASE("deals 60 damage") {
        CHECK(r.value == 60);
    }
    SUBCASE("returns damage type") {
        CHECK(r.type == ActionResult::Type::Damage);
    }
    SUBCASE("resets energy to zero") {
        CHECK(p.getEnergy() == 0);
    }
    SUBCASE("grants two SP but caps at kMaxSp") {
        CHECK(p.getSp() == Player::kMaxSp);
    }
}

TEST_CASE("Player::SP cap") {
    Player p{ "Rina" };
    for (int i{ 0 }; i < 20; ++i) p.basicAttack();
    CHECK(p.getSp() == Player::kMaxSp);
}

TEST_CASE("Player::energy cap") {
    Player p{ "Rina" };
    for (int i{ 0 }; i < 20; ++i) p.basicAttack();
    CHECK(p.getEnergy() == Player::kMaxEnergy);
}

TEST_CASE("Player::ultimateReady") {
    Player p{ "Rina" };

    SUBCASE("false before energy is full") {
        CHECK(p.ultimateReady() == false);
    }
    SUBCASE("true when energy reaches max") {
        for (int i{ 0 }; i < 5; ++i) p.basicAttack();
        CHECK(p.ultimateReady() == true);
    }
    SUBCASE("false again after ultimate is used") {
        for (int i{ 0 }; i < 5; ++i) p.basicAttack();
        p.useUltimate();
        CHECK(p.ultimateReady() == false);
    }
}