#pragma once
#include <cstddef>

struct TargetInfo
{
    enum class Type { Enemy, Ally, Self };
    Type        type{ Type::Enemy };
    std::size_t index{ 0 };   // position in the corresponding Party's unit container
};