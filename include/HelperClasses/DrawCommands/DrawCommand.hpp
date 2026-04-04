#pragma once

#include "Interfaces/DisplayTypes.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // DrawCommand
    // -------------------------------------------------------------------------
    // Interface for all draw commands. Constructed inline at the call site —
    // not intended to be stored and reused across frames.
    //
    // Implement draw(DrawContext&) to render anything via Adafruit_GFX.
    // DrawContext provides the display pointer, width, and height so commands
    // can adapt to any display size without compile-time constants.

    class DrawCommand
    {
    public:
        virtual ~DrawCommand() = default;
        virtual void draw(DrawContext &ctx) = 0;
    };

} // namespace DisplayModule
