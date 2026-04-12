#pragma once
#include <cstddef>

class Unit;
class Party;

// Decision returned by an AI strategy.
struct AIDecision
{
    std::size_t targetIndex{ 0 };   // index in the target Party
};

class IAIStrategy
{
public:
    virtual ~IAIStrategy() = default;
    virtual AIDecision decide(const Unit& self, const Party& targets) = 0;
};

// -----------------------------------------------------------------------
// Default Phase-1 AI: always targets the first alive unit in the party.
// -----------------------------------------------------------------------
class BasicAIStrategy : public IAIStrategy
{
public:
    AIDecision decide(const Unit& /*self*/, const Party& targets) override;
};