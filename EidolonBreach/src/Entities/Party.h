#pragma once

/**
 * @file Party.h
 * @brief Container for a group of Units and shared party resources.
 */

#include "Entities/Unit.h"
#include "Core/PartyResources.h"
#include "Items/Inventory.h"
#include <vector>
#include <memory>
#include <cstddef>

 /** Collection of Units (player party or enemy group). */
class Party {
public:
	void addUnit(std::unique_ptr<Unit> unit);

	bool               isAllDead() const;
	std::vector<Unit*> getAliveUnits() const;
	Unit* getUnitAt(std::size_t index);
	const Unit* getUnitAt(std::size_t index) const;
	std::size_t        size() const;
	bool               contains(const Unit* unit) const;
	std::size_t        getIndex(const Unit* unit) const;

	// Shared SP pool management
	int  getSp() const { return m_resources.sp; }
	int  getMaxSp() const { return m_resources.maxSp; }
	void gainSp(int amount);
	bool useSp(int amount);   // returns true if enough SP was available/** @return The shared party inventory (consumables, equipment, gold). */
    [[nodiscard]] Inventory &getInventory()
    {
        return m_inventory;
    }
    [[nodiscard]] const Inventory &getInventory() const
    {
        return m_inventory;
    }


private:
	std::vector<std::unique_ptr<Unit>> m_units;
	PartyResources m_resources{ 0, kDefaultMaxSp };

	static constexpr int kDefaultMaxSp{ 100 };
    Inventory m_inventory{};
};