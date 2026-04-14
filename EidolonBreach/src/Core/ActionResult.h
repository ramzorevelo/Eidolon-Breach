#pragma once

/** 
 * @file ActionResult.h 
 * @brief Result of an action used for UI and game state updates.
 */

#include <string>

struct ActionResult
{
    enum class Type { Damage, Heal, Charge, Skip };
    Type        type{ Type::Damage };
    int         value{ 0 };
    std::string flavorText{};  
};