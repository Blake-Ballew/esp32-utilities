#pragma once

#include <functional>
#include "DrawCommand.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // FnDrawCommand
    // -------------------------------------------------------------------------
    // Draw command backed by a std::function — useful for states that need to
    // capture local variables and render them inline rather than defining a
    // dedicated DrawCommand subclass.
    //
    // The lambda receives a DrawContext& so it can access the display pointer,
    // width, and height.  It must NOT call display->display() — that is done
    // by the display manager after all layers have drawn.
    //
    // Example:
    //   addDrawCommand(std::make_shared<FnDrawCommand>([this](DrawContext &ctx) {
    //       ctx.display->setCursor(0, 0);
    //       ctx.display->print(_someField.c_str());
    //   }));

    class FnDrawCommand : public DrawCommand
    {
    public:
        using Fn = std::function<void(DrawContext &)>;

        explicit FnDrawCommand(Fn fn) : _fn(std::move(fn)) {}

        void draw(DrawContext &ctx) override
        {
            if (_fn) _fn(ctx);
        }

    private:
        Fn _fn;
    };

} // namespace DisplayModule
