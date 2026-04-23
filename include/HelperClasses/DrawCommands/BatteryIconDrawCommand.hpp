#pragma once

#include "DrawCommand.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // BatteryIconDrawCommand
    // -------------------------------------------------------------------------
    // Draws a 14×8 px battery icon at (x, y).  The interior is filled
    // proportionally to 'percentage' (0–100).
    //
    //  ┌──────────┐╗
    //  │████░░░░░░│║  (filled portion proportional to percentage)
    //  └──────────┘╝

    class BatteryIconDrawCommand : public DrawCommand
    {
    public:
        BatteryIconDrawCommand(int16_t x, int16_t y, uint8_t percentage)
            : _x(x), _y(y), _pct(percentage > 100 ? 100 : percentage)
        {
        }

        void draw(DrawContext &ctx) override
        {
            // Outer rectangle (12×8)
            ctx.display->drawRect(_x, _y, 12, 8, DrawColorPrimary());
            // Fill bar proportional to charge level (max fill = 10px)
            if (_pct > 0)
                ctx.display->fillRect(_x + 1, _y + 1, _pct / 10, 6, DrawColorPrimary());
            // Terminal nub (2×4 on the right)
            ctx.display->fillRect(_x + 12, _y + 2, 2, 4, DrawColorPrimary());
        }

    private:
        int16_t _x, _y;
        uint8_t _pct;
    };

} // namespace DisplayModule
