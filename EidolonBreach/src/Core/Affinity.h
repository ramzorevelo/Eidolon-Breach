#pragma once

/** 
 * @file Affinity.h
 * @brief The five elemental affinities.
 */

#include <string>


enum class Affinity { Blaze, Frost, Tempest, Terra, Aether };

inline std::string affinityToString(Affinity a)
{
    switch (a)
    {
    case Affinity::Blaze:   return "Blaze";
    case Affinity::Frost:   return "Frost";
    case Affinity::Tempest: return "Tempest";
    case Affinity::Terra:   return "Terra";
    case Affinity::Aether:  return "Aether";
    default:                return "Unknown";
    }
}