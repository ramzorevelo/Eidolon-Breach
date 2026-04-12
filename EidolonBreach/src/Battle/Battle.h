#pragma once
#include "Entities/Party.h"
#include <vector>

class Battle
{
public:
    Battle(Party& playerParty, Party& enemyParty);
    void run();

private:
    Party& m_playerParty;
    Party& m_enemyParty;

    // ── Turn order helpers ────────────────────────────────────────────
    struct TurnSlot
    {
        Unit* unit;
        bool        isPlayer;     // true = belongs to player party
        std::size_t partyIndex;
    };

    std::vector<TurnSlot> buildTurnOrder() const;

    // Snapshots isBroken() for every unit in the party.
    std::vector<bool> snapshotBreakStates(const Party& party) const;

    // Renders renderBreak for units that transitioned false → true.
    void renderNewBreaks(const std::vector<bool>& before, const Party& party) const;
};