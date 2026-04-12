#pragma once

struct Stats
{
    int hp{};
    int maxHp{};
    int atk{};   // base attack value; flat bonuses from items add to this (Phase 3)
    int def{};   // used in DEF-reduction damage formula
    int spd{};   // determines turn order
};