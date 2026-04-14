#pragma once

/**
 * @file TurnSlot.h
 * @brief A single entry in the turn order.
 */#include <cstddef>

class Unit;

/** Represents one unit's upcoming turn. */
struct TurnSlot
{
    Unit *unit{nullptr};
    bool isPlayer{false}; // true = belongs to player party
    std::size_t partyIndex{0};
};