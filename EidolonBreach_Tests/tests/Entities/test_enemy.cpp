#include "doctest.h"
#include "Entities/Enemy.h"
#include "Core/Drop.h"

TEST_CASE("Enemy without a drop has no drop") {
    Enemy e{ "Dummy", 100, 50 };
    CHECK(!e.hasDrop());
}

TEST_CASE("Enemy with a drop reports it correctly") {
    Drop d{ "Iron Shard", 10 };
    Enemy e{ "Golem", 200, 100, d };

    SUBCASE("hasDrop returns true") {
        CHECK(e.hasDrop());
    }
    SUBCASE("getDrop returns correct name") {
        CHECK(e.getDrop()->name == "Iron Shard");
    }
    SUBCASE("getDrop returns correct gold value") {
        CHECK(e.getDrop()->goldValue == 10);
    }
}

TEST_CASE("Enemy drop is accessible after enemy dies") {
    Drop d{ "Iron Shard", 10 };
    Enemy e{ "Golem", 200, 100, d };
    e.takeDamage(999);
    CHECK(!e.isAlive());
    CHECK(e.hasDrop());
    CHECK(e.getDrop()->name == "Iron Shard");
}

TEST_CASE("Drop with empty name is still valid") {
    Drop d{ "", 5 };
    Enemy e{ "Golem", 200, 100, d };
    CHECK(e.hasDrop());
    CHECK(e.getDrop()->goldValue == 5);
}

TEST_CASE("Drop can have zero gold value") {
    Drop d{ "Broken Sword", 0 };
    Enemy e{ "Golem", 200, 100, d };
    CHECK(e.hasDrop());
    CHECK(e.getDrop()->goldValue == 0);
}

TEST_CASE("Enemy::dropLoot returns and removes the drop") {
    Drop d{ "Iron Shard", 10 };
    Enemy e{ "Golem", 200, 100, d };

    SUBCASE("returns the correct drop") {
        auto dropped = e.dropLoot();
        CHECK(dropped.has_value());
        CHECK(dropped->name == "Iron Shard");
        CHECK(dropped->goldValue == 10);
    }

    SUBCASE("removes the drop so hasDrop becomes false") {
        e.dropLoot();
        CHECK(!e.hasDrop());
    }

    SUBCASE("returns nullopt when there is no drop") {
        Enemy noDrop{ "Dummy", 100, 50 };
        CHECK(!noDrop.dropLoot().has_value());
    }
}


TEST_CASE("Enemy initialises with full toughness") {
    const int maxToughness{ 50 };
    Enemy e{ "Dummy", 100, maxToughness };

    CHECK(e.getToughness() == maxToughness);
    CHECK(e.getMaxToughness() == maxToughness);
    CHECK(e.isBroken() == false);
}

TEST_CASE("Enemy::reduceToughness") {
    const int maxToughness{ 50 };
    const int reduction{ 20 };
    Enemy e{ "Dummy", 100, maxToughness };

    SUBCASE("reduces toughness by given amount") {
        e.reduceToughness(reduction);
        CHECK(e.getToughness() == maxToughness - reduction);
    }
    SUBCASE("not broken before gauge empties") {
        e.reduceToughness(maxToughness - 1);
        CHECK(e.isBroken() == false);
    }
    SUBCASE("breaks when gauge reaches exactly zero") {
        e.reduceToughness(maxToughness);
        CHECK(e.isBroken() == true);
    }
    SUBCASE("breaks on overkill") {
        e.reduceToughness(maxToughness * 10);
        CHECK(e.isBroken() == true);
    }
    SUBCASE("gauge resets to max after break") {
        e.reduceToughness(maxToughness);
        CHECK(e.getToughness() == maxToughness);
    }
}

TEST_CASE("Enemy::recoverFromBreak") {
    const int maxToughness{ 50 };
    Enemy e{ "Dummy", 100, maxToughness };

    SUBCASE("clears broken flag") {
        e.reduceToughness(maxToughness);
        REQUIRE(e.isBroken() == true);
        e.recoverFromBreak();
        CHECK(e.isBroken() == false);
    }
    SUBCASE("toughness stays at max after recovery") {
        e.reduceToughness(maxToughness);
        e.recoverFromBreak();
        CHECK(e.getToughness() == maxToughness);
    }
    SUBCASE("calling recover when not broken has no effect") {
        e.recoverFromBreak();
        CHECK(e.isBroken() == false);
        CHECK(e.getToughness() == maxToughness);
    }
}

TEST_CASE("Enemy::performAttack returns damage") {
    Enemy e{ "Dummy", 100, 50 };
    ActionResult r = e.performAttack();

    CHECK(r.type == ActionResult::Type::Damage);
    CHECK(r.value == 20);
}