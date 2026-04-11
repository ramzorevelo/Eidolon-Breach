#include "doctest.h"
#include "Entities/Character.h"

TEST_CASE("Character initialises with full HP") {
    const int maxHp{ 100 };
    Character c{ "Hero", maxHp };

    CHECK(c.getHp() == maxHp);
    CHECK(c.getMaxHp() == maxHp);
    CHECK(c.isAlive() == true);
}

TEST_CASE("Character::takeDamage") {
    const int maxHp{ 100 };
    const int damage{ 30 };
    Character c{ "Hero", maxHp };

    SUBCASE("reduces HP by given amount") {
        c.takeDamage(damage);
        CHECK(c.getHp() == maxHp - damage);
    }
    SUBCASE("clamps HP to zero on overkill") {
        c.takeDamage(maxHp * 10);
        CHECK(c.getHp() == 0);
    }
    SUBCASE("zero damage has no effect") {
        c.takeDamage(0);
        CHECK(c.getHp() == maxHp);
    }
}

TEST_CASE("Character::heal") {
    const int maxHp{ 100 };
    const int damage{ 50 };
    const int healAmount{ 20 };
    Character c{ "Hero", maxHp };
    c.takeDamage(damage);

    SUBCASE("restores HP by given amount") {
        c.heal(healAmount);
        CHECK(c.getHp() == maxHp - damage + healAmount);
    }
    SUBCASE("clamps HP to maxHp") {
        c.heal(maxHp * 10);
        CHECK(c.getHp() == maxHp);
    }
    SUBCASE("zero heal has no effect") {
        c.heal(0);
        CHECK(c.getHp() == maxHp - damage);
    }
}

TEST_CASE("Character::isAlive") {
    const int maxHp{ 100 };

    SUBCASE("returns true when HP is above zero") {
        Character c{ "Hero", maxHp };
        CHECK(c.isAlive() == true);
    }
    SUBCASE("returns false when HP is exactly zero") {
        Character c{ "Hero", maxHp };
        c.takeDamage(maxHp);
        CHECK(c.isAlive() == false);
    }
    SUBCASE("returns false after overkill") {
        Character c{ "Hero", maxHp };
        c.takeDamage(maxHp * 10);
        CHECK(c.isAlive() == false);
    }
}

TEST_CASE("Character::getName") {
    SUBCASE("returns the name it was constructed with") {
        Character c{ "Hero", 100 };
        CHECK(c.getName() == "Hero");
    }
    SUBCASE("handles empty string name") {
        Character c{ "", 100 };
        CHECK(c.getName() == "");
    }
    SUBCASE("handles name with spaces") {
        Character c{ "Dark Knight", 100 };
        CHECK(c.getName() == "Dark Knight");
    }
    SUBCASE("handles special characters") {
        Character c{ "-[Eldritch_Void-Walker#99]-", 100 };
        CHECK(c.getName() == "-[Eldritch_Void-Walker#99]-");
    }
    SUBCASE("handles very long name") {
        std::string longName(1000, 'a');
        Character c{ longName, 100 };
        CHECK(c.getName() == longName);
    }
}