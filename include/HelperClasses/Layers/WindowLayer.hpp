#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include "LayerInterface.hpp"
#include "DisplayUtilities.hpp"
#include "HelperClasses/Window/WindowState.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // WindowLayer  (LayerID::WINDOW = 1)
    // -------------------------------------------------------------------------
    // Renders in two passes:
    //   1. Window::_drawCommands — persistent chrome (borders, titles, icons)
    //   2. Input labels/indicators — driven by a per-inputID Factory map.
    //
    // Factory system
    // --------------
    // Each inputID is paired with a Factory:
    //   using Factory = std::function<void(DrawContext &, const std::string &)>
    //
    // The factory receives:
    //   - ctx   — draw context (display, width, height)
    //   - label — the text registered via bindInput() or registerInput()
    //             Empty for encoder bindings; the factory may ignore it.
    //
    // Default factories for the 6 canonical input IDs:
    //   BUTTON_1 (1) — text at bottom-left  corner
    //   BUTTON_2 (2) — text at bottom-right corner
    //   BUTTON_3 (3) — text at top-left     corner
    //   BUTTON_4 (4) — text at top-right    corner
    //   ENC_UP   (5) — "^" arrow at top-center    (label ignored)
    //   ENC_DOWN (6) — "v" arrow at bottom-center  (label ignored)
    //
    // To add or replace a factory for a custom input:
    //   windowLayer->registerFactory(MyID, [](DrawContext &ctx, const std::string &lbl) {
    //       ctx.display->setCursor(...);
    //       ctx.display->print(lbl.c_str());
    //   });
    //
    // Label resolution
    // ----------------
    // For each rendering pass the layer collects all active input bindings:
    //   Priority 1: Active state's bindInput() bindings (set per-state in onEnter)
    //   Priority 2: Window-level registerInput() labels (fallback/default)
    //
    // States use bindInput(ENC_UP, "") to activate the scroll-arrow factory
    // without providing any label text.  States no longer need to manually add
    // "^"/"v" TextDrawCommands — the factory handles placement automatically.
    //
    // A state may pass an empty label for a button inputID to suppress a
    // window-level label while that state is active.

    class WindowLayer : public LayerInterface
    {
    public:
        using Factory = std::function<void(DrawContext &, const std::string &)>;

        WindowLayer()
        {
        }

        // Register (or replace) the factory for a given inputID.
        // Call from application startup for custom hardware inputs.
        void registerFactory(uint8_t inputID, Factory fn)
        {
            _factories[inputID] = std::move(fn);
        }

        // ------------------------------------------------------------------
        // LayerInterface
        // ------------------------------------------------------------------

        void draw(DrawContext &ctx) override
        {
            if (!_enabled) {
                ESP_LOGV(TAG, "WindowLayer is disabled; skipping draw.");
                return;
            }

            auto win = Utilities::activeWindow();
            if (!win) 
            {
                ESP_LOGW(TAG, "WindowLayer has no active window; skipping draw.");
                return;
            }

            ESP_LOGV(TAG, "Drawing WindowLayer for active window with %d draw commands",
                     static_cast<int>(win->drawCommands().size()));

            // Pass 1 — window-level draw commands (persistent chrome)
            for (auto &cmd : win->drawCommands())
                cmd->draw(ctx);

            // Pass 2 — input labels/indicators via factory system
            drawInputLabels(ctx, *win);  // *win is WindowInterface &
        }

    protected:
        std::unordered_map<uint8_t, Factory> _factories;

        // ------------------------------------------------------------------
        // drawInputLabels
        // ------------------------------------------------------------------
        // Collects all active bindings (state overrides window), then calls
        // the registered factory for each inputID that has one.
        //
        // Override in a subclass to completely replace label rendering.
        // ------------------------------------------------------------------
        virtual void drawInputLabels(DrawContext &ctx, WindowInterface &win)
        {
            auto state = win.currentState(); // may be nullptr

            // ----------------------------------------------------------------
            // Build a set of (inputID → label) with state taking precedence.
            // We track which IDs are "handled" using a bitmask so we only
            // iterate each ID once.  InputIDs are guaranteed < 64 (USER_BASE=16,
            // application IDs rarely exceed 32).
            // ----------------------------------------------------------------
            uint64_t handled = 0; // bitmask of processed input IDs

            // ---- State bindings (highest priority) ----
            if (state)
            {
                for (auto &[id, binding] : state->inputBindings())
                {
                    auto fit = _factories.find(id);
                    if (fit != _factories.end())
                        fit->second(ctx, binding.label);

                    if (id < 64)
                        handled |= (uint64_t(1) << id);
                }
            }

            // ---- Window-level inputs (fallback for IDs not bound by state) ----
            for (auto &[id, entry] : win.inputs())
            {
                if (id < 64 && (handled & (uint64_t(1) << id)))
                    continue; // already handled by state

                auto fit = _factories.find(id);
                if (fit != _factories.end())
                    fit->second(ctx, entry.label);
            }
        }

    private:
    };

} // namespace DisplayModule
