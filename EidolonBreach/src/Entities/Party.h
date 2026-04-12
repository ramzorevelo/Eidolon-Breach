#pragma once
#include "Entities/Unit.h"
#include <vector>
#include <memory>
#include <cstddef>
/**
 * @file Party.h
 * @brief Container for a group of Units.
 */

/** Collection of Units (player party or enemy group). */
class Party
{
public:
    void addUnit(std::unique_ptr<Unit> unit);

    bool               isAllDead()                        const;
    std::vector<Unit*> getAliveUnits()                    const;
    Unit* getUnitAt(std::size_t index);
    const Unit* getUnitAt(std::size_t index)       const;
    std::size_t        size()                             const;
    bool               contains(const Unit* unit)         const;
    std::size_t        getIndex(const Unit* unit)         const;  

private:
    std::vector<std::unique_ptr<Unit>> m_units;
};