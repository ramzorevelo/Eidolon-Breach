#pragma once
#include <cstddef>

/** 
 * @file TargetInfo.h 
 * @brief Target selection data passed to IAction. 
 */
struct TargetInfo
{
    enum class Type { Enemy, Ally, Self };
    Type        type{ Type::Enemy };
    std::size_t index{ 0 };   
};