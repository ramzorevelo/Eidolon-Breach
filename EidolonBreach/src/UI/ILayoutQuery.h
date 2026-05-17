#pragma once
/**
 * @file ILayoutQuery.h
 * @brief Narrow interface for hit-test and log-control queries.
 *        Implemented by SDL3Renderer; consumed only by SDL3InputHandler.
 *        Separates layout-geometry concerns from rendering concerns.
 */

class ILayoutQuery
{
  public:
    virtual ~ILayoutQuery() = default;

    [[nodiscard]] virtual int getActionRowAt(int x, int y) const = 0;
    [[nodiscard]] virtual int getUnitCardAt(int x, int y, bool isPlayerSide) const = 0;
    virtual void setLogScrollOffset(int delta) = 0;
    virtual void expandLog(bool expand) = 0;
    [[nodiscard]] virtual bool isLogScrollable() const = 0;
    [[nodiscard]] virtual bool isHighlightingEnemies() const = 0;
    [[nodiscard]] virtual int getWindowHeight() const = 0;
    [[nodiscard]] virtual int getMenuRowAt(int x, int y) const = 0;
};