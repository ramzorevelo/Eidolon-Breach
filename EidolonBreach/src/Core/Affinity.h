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

/**
 * @brief Returns the affinity that is weak to the given affinity.
 *        Used for floor environmental toughness modifiers.
 *        Aether has no opponent and returns Aether.
 */
[[nodiscard]] inline Affinity getAffinityOpponent(Affinity a)
{
    switch (a)
    {
    case Affinity::Blaze:
        return Affinity::Frost; // Frost is weak to Blaze
    case Affinity::Frost:
        return Affinity::Tempest; // Tempest is weak to Frost
    case Affinity::Tempest:
        return Affinity::Terra; // Terra is weak to Tempest
    case Affinity::Terra:
        return Affinity::Blaze; // Blaze is weak to Terra
    case Affinity::Aether:
        return Affinity::Aether;
    default:
        return Affinity::Aether;
    }
}