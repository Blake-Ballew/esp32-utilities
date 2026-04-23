#pragma once

#include "DrawCommand.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // MessageIconDrawCommand
    // -------------------------------------------------------------------------
    // Draws a 12×8 px filled envelope / message icon at (x, y).
    //
    //  ┌──────────┐
    //  │\        /│   (diagonal lines form the envelope flap)
    //  └──────────┘

    class MessageIconDrawCommand : public DrawCommand
    {
    public:
        MessageIconDrawCommand(int16_t x, int16_t y) : _x(x), _y(y) {}

        void draw(DrawContext &ctx) override
        {
            ESP_LOGV(TAG, "Drawing message icon at %d, %d", _x, _y);
            // Filled rectangle body
            ctx.display->fillRect(_x, _y, 12, 8, DrawColorPrimary());
            // Envelope flap — two black diagonal lines
            ctx.display->drawLine(_x,      _y,     _x + 5,  _y + 3, BLACK);
            ctx.display->drawLine(_x + 6,  _y + 3, _x + 11, _y,     BLACK);
        }

    private:
        int16_t _x, _y;
    };

} // namespace DisplayModule
