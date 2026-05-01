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
#include <optional>
#include <cstddef>
#include "Vestiges/IVestige.h"
class IVestige;
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
    /**
     * @brief Remove the unit with the given ID from the party.
     *        No-op if the ID is not found. Ownership is released and destroyed.
     */
    void removeUnit(std::string_view unitId);
    /**
     * @brief Remove the vestige at the given index (0-based).
     *        No-op when index is out of range.
     * @return true if a vestige was removed.
     */
    bool removeVestige(std::size_t index);

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
    static constexpr int kMaxVestiges{5};

    /**
     * @brief Attempt to add a vestige.
     * @return nullopt if added successfully. Returns the vestige back if at kMaxVestiges
     *         capacity so the caller can handle the overflow (e.g. offer discard).
     */
    std::optional<std::unique_ptr<IVestige>> addVestige(std::unique_ptr<IVestige> vestige);

    /** @return Read-only view of all held vestiges. */
    [[nodiscard]] const std::vector<std::unique_ptr<IVestige>> &getVestiges() const;

private:
	std::vector<std::unique_ptr<Unit>> m_units;
	PartyResources m_resources{ 0, kDefaultMaxSp };

	static constexpr int kDefaultMaxSp{ 100 };
    Inventory m_inventory{};
    std::vector<std::unique_ptr<IVestige>> m_vestiges{};
};