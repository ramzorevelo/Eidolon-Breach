#pragma once
#include <cstddef>

class Unit;
class Party;

// Decision returned by an AI strategy.
struct AIDecision
{
    std::size_t targetIndex{ 0 };   
};

class IAIStrategy
{
public:
    virtual ~IAIStrategy() = default;
    virtual AIDecision decide(const Unit& self, const Party& targets) = 0;
};

/** Basic AI that targets the first alive unit in the given party. */
class BasicAIStrategy : public IAIStrategy
{
public:
    AIDecision decide(const Unit& /*self*/, const Party& targets) override;
};