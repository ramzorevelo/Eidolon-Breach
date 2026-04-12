#pragma once
#include <string>

struct ActionResult
{
    enum class Type { Damage, Heal, Charge, Skip };
    Type        type{ Type::Damage };
    int         value{ 0 };
    std::string flavorText{};   // optional custom message (e.g., "GROUND SLAM!")
};