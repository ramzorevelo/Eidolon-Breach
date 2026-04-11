#include "Battle/Battle.h"
#include "Entities/VampireBat.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main()
{
    std::cout << "Enter your name: ";
    std::string name{};
    std::getline(std::cin, name);
    if (name.empty())
        name = "Hero";

    Player player{ name };
    VampireBat bat{ "Vampire Bat", 100, 40 };

    std::vector<std::unique_ptr<IAction>> actions;
    actions.push_back(std::make_unique<BasicStrikeAction>());
    actions.push_back(std::make_unique<SkillAction>());
    actions.push_back(std::make_unique<UltimateAction>());

    Battle battle{ player, bat, std::move(actions) };
    battle.run();

    return 0;
}