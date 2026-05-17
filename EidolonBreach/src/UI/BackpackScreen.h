#pragma once
/**
 * @file BackpackScreen.h
 * @brief Read-only SDL3-rendered inventory view.
 *        Three tabs: Consumables, Equipment, Key Items.
 *        Key Items tab is a stub. Persistent inventory wiring is deferred.
 */

class Inventory;
class SDL3Renderer;
class SDL3InputHandler;

class BackpackScreen
{
  public:
    /**
     * @brief Run the backpack screen. Blocks until the player exits (Esc or Back).
     * @param inventory Inventory to display (read-only).
     * @param renderer  SDL3 renderer.
     * @param input     SDL3 input handler.
     */
    static void run(const Inventory &inventory,
                    SDL3Renderer &renderer,
                    SDL3InputHandler &input);

  private:
    enum class Tab
    {
        Consumables,
        Equipment,
        KeyItems
    };

    static void showTab(Tab tab,
                        const Inventory &inventory,
                        SDL3Renderer &renderer,
                        SDL3InputHandler &input);
};