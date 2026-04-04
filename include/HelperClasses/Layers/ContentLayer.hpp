#pragma once

#include "LayerInterface.hpp"
#include "DisplayUtilities.hpp"
#include "HelperClasses/Window/WindowState.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // ContentLayer  (LayerID::CONTENT = 0)
    // -------------------------------------------------------------------------
    // Renders the active state's draw command vector.
    // Drawn first — lowest visual priority.
    //
    // The active state populates its _drawCommands vector in onEnter() and
    // clears it in onExit(). This layer simply iterates and calls each one.

    class ContentLayer : public LayerInterface
    {
    public:
        void draw(DrawContext &ctx) override
        {
            if (!_enabled) 
            {
                ESP_LOGV(TAG, "ContentLayer is disabled; skipping draw.");
                return;
            }

            auto win = Utilities::activeWindow();
            if (!win) 
            {
                ESP_LOGW(TAG, "ContentLayer has no active window; skipping draw.");
                return;
            }

            auto state = win->currentState();
            if (!state) 
            {
                ESP_LOGW(TAG, "ContentLayer has no active state; skipping draw.");
                return;
            }

            ESP_LOGV(TAG, "Drawing ContentLayer for active state with %d draw commands",
                     static_cast<int>(state->drawCommands().size()));

            for (auto &cmd : state->drawCommands())
                cmd->draw(ctx);
        }
    };

} // namespace DisplayModule
