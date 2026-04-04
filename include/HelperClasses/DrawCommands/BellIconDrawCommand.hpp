#pragma once

#include "DrawCommand.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // BellIconDrawCommand
    // -------------------------------------------------------------------------
    // Draws an 11×8 px bell icon at (x, y).
    // When isSilent is true, draws a diagonal slash through the bell.

    class BellIconDrawCommand : public DrawCommand
    {
    public:
        BellIconDrawCommand(int16_t x, int16_t y, bool isSilent)
            : _x(x), _y(y), _silent(isSilent)
        {
        }

        void draw(DrawContext &ctx) override
        {
            // Bell dome
            ctx.display->fillCircle(_x + 5, _y + 2, 3, WHITE);
            // Bell body
            ctx.display->fillRect(_x + 2, _y + 2, 7, 6, WHITE);
            // Bell skirt (triangle pointing up = bottom of bell)
            ctx.display->fillTriangle(
                _x,      _y + 7,
                _x + 10, _y + 7,
                _x + 5,  _y + 2,
                WHITE);

            if (_silent)
            {
                // Diagonal slash indicating muted
                ctx.display->drawLine(_x,     _y + 5, _x + 8, _y + 1, BLACK);
                ctx.display->drawLine(_x,     _y + 6, _x + 8, _y + 2, BLACK);
            }
        }

    private:
        int16_t _x, _y;
        bool    _silent;
    };

} // namespace DisplayModule
