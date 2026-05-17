#pragma once
/**
 * @file SDL3Renderer.h
 * @brief Resolution-agnostic SDL3 renderer.
 *
 * All drawing uses a fixed 1280×720 logical canvas. The SDL3
 * logical presentation mode LETTERBOX maintains aspect ratio.
 * Mouse input is converted via SDL_RenderCoordinatesFromWindow.
 */

#include "UI/ILayoutQuery.h"
#include "UI/IRenderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct TurnSlot;
class Party;
class PlayableCharacter;
class ResonanceField;
enum class Affinity;

class SDL3Renderer : public IRenderer, public ILayoutQuery
{
  public:
    SDL3Renderer(const char *windowTitle, int initialWidth, int initialHeight);
    ~SDL3Renderer() override;

    SDL3Renderer(const SDL3Renderer &) = delete;
    SDL3Renderer &operator=(const SDL3Renderer &) = delete;
    SDL3Renderer(SDL3Renderer &&) = delete;
    SDL3Renderer &operator=(SDL3Renderer &&) = delete;

    // IRenderer
    void renderActionResult(const std::string &actorName,
                            const ActionResult &result) override;
    void renderBreak(const std::string &enemyName) override;
    void renderStunned(const std::string &enemyName) override;
    void renderVictory(const std::string &enemyName,
                       std::optional<Drop> drop) override;
    void renderDefeat(const std::string &playerName) override;
    void renderPartyStatus(const Party &playerParty,
                           const Party &enemyParty) override;
    void renderMessage(const std::string &message) override;
    void renderResonanceField(const ResonanceField &field) override;
    void renderActionMenu(const PlayableCharacter &character,
                          const Party &party) override;
    void renderTargetList(const std::vector<std::string> &names,
                          bool isAllyTarget = false) override;
    void renderTurnOrder(const std::vector<TurnSlot> &order) override;
    void renderHintBar(const std::string &hint) override;
    void clearTargetHighlight() override;
    void updateTargetHighlight(int index) override;
    void presentPause(int ms) override;
    void renderSelectionMenu(const std::string &title,
                             const std::vector<std::string> &options,
                             std::size_t selected = 0) override;
    void clearBattleCache();
    void renderDungeonSelect(const std::string &title,
                             const std::vector<DungeonSelectInfo> &dungeons,
                             std::size_t selected = 0) override;

    // ILayoutQuery
    [[nodiscard]] int getActionRowAt(int x, int y) const override;
    [[nodiscard]] int getUnitCardAt(int x, int y, bool isPlayerSide) const override;
    void setLogScrollOffset(int delta) override;
    void expandLog(bool expand) override;
    [[nodiscard]] bool isLogScrollable() const override;
    [[nodiscard]] bool isHighlightingEnemies() const override
    {
        return m_highlightingEnemies;
    }
    [[nodiscard]] int getWindowHeight() const override
    {
        return m_windowHeight;
    }
    [[nodiscard]] int getMenuRowAt(int x, int y) const override;

    void renderTooltip(const std::string &name, float hpFraction,
                       const std::string &effectSummary, int screenX, int screenY);
    void onWindowResized(int newWidth, int newHeight);

    /**
     * @brief Programmatically resize the window and update internal tracking.
     *        Called by SettingsScreen when the player picks a new resolution.
     *        The logical canvas stays at 1280x720; SDL letterboxing handles scaling.
     * @param w New window width in physical pixels.
     * @param h New window height in physical pixels.
     */
    void setResolution(int w, int h);

    void flushAnimations();
    void flushVisualEffects() override;

  private:
    // Card geometry constants shared between draw functions and getUnitCardAt.
    // A single source of truth prevents the hit-test geometry drifting from
    // the drawn geometry when one side is updated.
    // Inner top padding applied inside every card so content never touches the card edge.
    static constexpr float kCardTopPad = 5.f;
    // Inner bottom padding between the last bar and the card's bottom edge.
    static constexpr float kCardBotPad = 4.f;
    // Gap between the name text and the HP bar beneath it.
    static constexpr float kNameBarGap = 4.f;

    static constexpr float kPCPortraitW = 44.f;
    static constexpr float kPCPortraitH = 44.f;
    static constexpr float kPCPortraitPad = 4.f;
    static constexpr float kPCNameH = 16.f;
    static constexpr float kPCBarH = 8.f;
    static constexpr float kPCThinH = 6.f;
    static constexpr float kPCGapH = 4.f;
    static constexpr float kPCBottomGap = 8.f; // gap between cards (not inside)

    static constexpr float kEPortraitW = 36.f;
    static constexpr float kEPortraitH = 36.f;
    static constexpr float kEPortraitPadR = 4.f;
    static constexpr float kENameH = 18.f;
    static constexpr float kEBarH = 8.f;
    static constexpr float kEThinH = 6.f;
    static constexpr float kEGapH = 3.f;
    static constexpr float kEIntentH = 14.f;
    static constexpr float kEBottomGap = 8.f; // gap between cards (not inside)

    SDL_Window *m_window{nullptr};
    SDL_Renderer *m_renderer{nullptr};
    TTF_Font *m_font{nullptr};       // 16 pt — UI labels and log
    TTF_Font *m_fontLarge{nullptr};  // 22 pt — portrait initials and overlays
    TTF_Font *m_fontDamage{nullptr}; // 20 pt — floating damage numbers

    int m_windowWidth{1280};
    int m_windowHeight{720};

    SDL_FRect m_turnOrderPanel{};
    SDL_FRect m_playerPanel{};
    SDL_FRect m_centerPanel{};
    SDL_FRect m_enemyPanel{};
    SDL_FRect m_actionMenu{};
    SDL_FRect m_logPanel{};
    SDL_FRect m_hintBar{};

    const Party *m_cachedPlayerParty{nullptr};
    const Party *m_cachedEnemyParty{nullptr};
    const PlayableCharacter *m_cachedActiveCharacter{nullptr};
    const Party *m_cachedActiveParty{nullptr};
    const ResonanceField *m_cachedResonanceField{nullptr};
    std::vector<TurnSlot> m_cachedTurnOrder{};
    std::string m_cachedHint{};

    struct LogEntry
    {
        std::string text;
        SDL_Color color{220, 220, 220, 255};
    };
    static constexpr int kMaxLogLines{64};
    std::vector<LogEntry> m_log{};
    int m_logScrollOffset{0};
    bool m_logExpanded{false};
    Uint64 m_lastLogTime{0};

    Affinity m_rfTriggerAffinity{};
    Uint64 m_rfTriggerExpiry{0};
    int m_cachedRfGauge{0};

    struct VisualBarState
    {
        float displayed{1.f};
        float target{1.f};
    };
    std::unordered_map<const Unit *, VisualBarState> m_hpBars;
    std::unordered_map<const Unit *, VisualBarState> m_toughnessBars;
    float m_rfGaugeDisplayed{0.f};
    float m_rfGaugeTarget{0.f};
    float m_rfGaugePulse{1.0f}; // temporary overshoot on trigger

    const Unit *m_breakFlashUnit{nullptr};
    Uint64 m_breakFlashExpiry{0};
    const Unit *m_deathFlashUnit{nullptr};
    Uint64 m_deathFlashExpiry{0};
    static constexpr int kDeathFlashDurationMs{400};

    struct DamageNumber
    {
        std::string text{};
        float spawnX{0.f}; // precomputed logical-space position
        float spawnY{0.f};
        Uint64 birthTime{0};
        float durationMs{1200.f};
        SDL_Color color{255, 100, 100, 255};
    };
    std::vector<DamageNumber> m_damageNumbers;

    // Damage texture cache — key encodes text + color to prevent cross-color reuse.
    std::unordered_map<std::string, SDL_Texture *> m_damageTextCache;

    // Text rendering cache — key is "text\0r,g,b". Cleared on clearBattleCache and destructor.
    std::unordered_map<std::string, SDL_Texture *> m_textCache;

    float m_stripSlideOffset{0.f};

    struct FractureLine
    {
        float startX{0.f}, startY{0.f};
        float angle{0.f};
        float length{30.f};
        Uint64 birthTime{0};
        Uint64 durationMs{600};
        SDL_Color color{255, 255, 255, 255};
    };
    std::vector<FractureLine> m_fractureLines;

    // Initialized to 0; assigned SDL_GetTicks() in the constructor body after SDL_Init.
    Uint64 m_lastFrameTicks{0};

    struct ShakeState
    {
        float intensity{0.f}; // peak x-axis offset amplitude in logical pixels
        Uint64 startMs{0};
        Uint64 durationMs{350};
    };
    std::unordered_map<const Unit *, ShakeState> m_shakes;

    /**
     * @brief Units that have already acted in the current (incomplete) cycle,
     *        in the order they acted. Pre-populated into the divider detection's
     *        "seen" set so cycle boundaries reflect battle history, not just the
     *        first repeat visible in the projected strip.
     *
     * Cleared when a full cycle completes (detected when the first unit of the
     * new projected strip is already in this set, meaning the fastest unit is
     * about to act again). Pointer validity: units are never destroyed during
     * battle, so these raw pointers are safe until clearBattleCache().
     */
    std::vector<const Unit *> m_actedThisCycle{};
    // Dungeon select state — cached so SDL3InputHandler's renderSelectionMenu
    // calls on Up/Down navigation re-draw the split layout automatically.
    std::vector<DungeonSelectInfo> m_dungeonSelectInfos{};
    std::string m_dungeonSelectTitle{};

    float m_menuPanelX{0.f};
    float m_menuPanelW{0.f};
    float m_menuFirstRowY{0.f};
    float m_menuRowStep{0.f};
    std::size_t m_menuWindowStart{0};
    std::size_t m_menuNumOptions{0};
    std::size_t m_menuNumVisible{0};


    void redrawAll();

    // Panel draws — read only from cached state; called only from redrawAll.
    void drawTurnOrderStrip();
    void drawBattleArea();
    void drawPlayerPanel();
    void drawEnemyPanel();
    void drawCenterPanel();
    void drawActionMenuPanel();
    void drawLogPanel();
    void drawHintBarPanel();

    /**
     * @brief Draw the dungeon selection split layout (list left, detail right).
     *        Called from renderDungeonSelect and from renderSelectionMenu when
     *        dungeon context is active.
     */
    void drawDungeonSelectScreen(const std::string &title,
                                 const std::vector<DungeonSelectInfo> &dungeons,
                                 std::size_t selected);

    /**
     * @brief Draw the right-hand detail panel for one dungeon.
     *        Called from drawDungeonSelectScreen.
     */
    void drawDungeonDetailPanel(const SDL_FRect &panel,
                                const DungeonSelectInfo &info);

    /**
     * @brief Draw the floor layout node strip inside the detail panel.
     * @param startX    Left edge of the first node card.
     * @param y         Top edge of the strip.
     * @param maxWidth  Maximum horizontal space available for the strip.
     */
    void drawFloorLayoutStrip(float startX, float y, float maxWidth,
                              const std::vector<std::string> &layout);
    void drawPlayerCard(const Unit *u, float &py, bool isActive, bool highlighted);
    void drawPlayerCardBars(const PlayableCharacter *pc, float barX, float barW,
                            float &py, bool isActive);
    void drawEnemyCard(const Unit *u, float &ey, bool highlighted);
    void drawFractureLines();
    void drawDamageNumbers();

    /**
     * @brief Screen-edge danger vignette, shown only when a PC is below 30 % HP.
     * @param severity  0 = invisible, 1 = full danger (0 % HP).
     *
     * Uses asymmetric horizontal/vertical insets on stacked rects to approximate
     * an elliptical falloff. Drawn after all UI panels so it overlays them slightly,
     * which is intentional for a screen-wide danger indicator.
     */
    void drawLowHpVignette(float severity);

    void computePanelLayout();

    // Primitives
    void fillRect(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void renderBorder(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    /**
     * @brief Fill a rect with 45° chamfered corners.
     * @param cut  Corner cut size in logical pixels.
     *
     * Implemented as two overlapping axis-aligned rects — no SDL_RenderGeometry required.
     */
    void renderChamferedFillRect(const SDL_FRect &rect, float cut,
                                 Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    /**
     * @brief Draw a 1 px chamfered (octagonal) border using SDL_RenderLines.
     *        Matches the corners of renderChamferedFillRect exactly.
     */
    void renderChamferedBorder(const SDL_FRect &rect, float cut,
                               Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    void renderBar(const SDL_FRect &rect, float fraction,
                   Uint8 fr, Uint8 fg, Uint8 fb,
                   Uint8 br, Uint8 bg, Uint8 bb);
    void renderText(const std::string &text, float x, float y,
                    Uint8 r, Uint8 g, Uint8 b);
    void renderTextEx(TTF_Font *font, const std::string &text,
                      float x, float y, Uint8 r, Uint8 g, Uint8 b);

    void advanceAnimations();
    void runAnimationFrames(int durationMs);
    [[nodiscard]] bool animationPending() const;
    [[nodiscard]] bool hasActiveVisualEffects() const;

    void fillRects(const std::vector<SDL_FRect> &rects,
                   Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    SDL_Texture *getOrCreateDamageTexture(const std::string &text, const SDL_Color &color);
    void clearDamageTextures();
    void clearTextCache();

    /**
     * @return Logical-space {centerX, topY} of the card for the named unit.
     *         Returns {fallbackX, fallbackY} if the unit is not found.
     */
    [[nodiscard]] std::pair<float, float>
    findCardCenter(const std::string &unitName, bool searchEnemySide,
                   float fallbackX, float fallbackY) const;

    /**
     * @brief Total height a player-side card occupies in the panel, in logical pixels.
     *        Must stay in sync with the geometry walked in drawPlayerCard.
     */
    [[nodiscard]] float playerCardHeight(const Unit *u, bool isActive) const;

    /**
     * @brief Total height an enemy-side card occupies in the panel, in logical pixels.
     *        Must stay in sync with the geometry walked in drawEnemyCard.
     */
    [[nodiscard]] float enemyCardHeight(const Unit *u) const;

    void addLogMessage(const std::string &msg, SDL_Color color = {220, 220, 220, 255});

    /** @return Horizontal shake offset for the unit's card this frame, or 0 if not shaking. */
    [[nodiscard]] float computeShakeOffset(const Unit *u) const;

    int m_highlightedTargetIndex{-1};
    bool m_highlightingEnemies{true};
    bool m_battleActive{false};

    static SDL_Color affinityColor(Affinity a);
};