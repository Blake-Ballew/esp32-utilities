#pragma once

// WindowFactories.hpp
// ---------------------------------------------------------------------------
// Convenience factory functions for the most common window configurations.
// Include this header instead of pulling in each window header individually.
//
// All factories return a shared_ptr<Window> (or a concrete subtype).
// They do NOT push the window onto the Utilities stack — call
// Utilities::pushWindow() yourself so call sites retain control over timing.
//
// Example:
//   auto menu = DisplayModule::makeMenuWindow({
//       { "Settings",  settingsState },
//       { "Calibrate", nullptr, [](Window &) { startCalibration(); } },
//   });
//   DisplayModule::Utilities::pushWindow(menu);

#include "DisplayUtilities.hpp"
#include "HelperClasses/Window/MenuWindow.hpp"
#include "HelperClasses/Window/SettingsWindow.hpp"
#include "HelperClasses/Window/States/TextDisplayState.hpp"
#include "HelperClasses/Window/States/ConfirmState.hpp"
#include "HelperClasses/Layers/ContentLayer.hpp"
#include "HelperClasses/Layers/WindowLayer.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // makeMenuWindow
    // -------------------------------------------------------------------------
    // Builds a MenuWindow pre-loaded with the given items.
    //
    // Each item is a struct with:
    //   label         — display string
    //   onSelect      — callback fired upon selection
    //
    // Usage:
    //   auto win = makeMenuWindow({
    //       { "Option A" , someFunction},
    //       { "Option B", [](Window &w) { doSomething(); } },
    //   });

    inline std::shared_ptr<MenuWindow>
    makeMenuWindow(std::initializer_list<MenuItem> items)
    {
        auto win = std::make_shared<MenuWindow>();
        for (auto &item : items)
            win->addItem(item);
        return win;
    }

    // Overload accepting a pre-built vector (useful when items are dynamic)
    inline std::shared_ptr<MenuWindow>
    makeMenuWindow(std::vector<MenuItem> items)
    {
        auto win = std::make_shared<MenuWindow>();
        for (auto &item : items)
            win->addItem(std::move(item));
        return win;
    }

    // -------------------------------------------------------------------------
    // makeTextWindow
    // -------------------------------------------------------------------------
    // Builds a bare Window hosting a TextDisplayState.
    // Wires BUTTON_3 ("Back") to pop the window.
    //
    // Usage — programmatic:
    //   auto win = makeTextWindow({
    //       TextDrawData("Hello", { TextAlignH::CENTER, TextAlignV::CENTER }),
    //   });
    //
    // Usage — payload-driven (lines supplied at push time via StateTransferData):
    //   auto win = makeTextWindow({});    // empty lines — filled by payload on enter
    //   Utilities::pushWindow(win);       // caller sends payload separately

    inline std::shared_ptr<Window>
    makeTextWindow(std::vector<TextDrawData> lines)
    {
        auto state = std::make_shared<TextDisplayState>();
        state->setLines(std::move(lines));

        auto win = std::make_shared<Window>();
        win->setInitialState(state);

        win->registerInput(InputID::BUTTON_3, "Back");
        win->addInputCommand(InputID::BUTTON_3,
            [](const InputContext &) { Utilities::popWindow(); });

        return win;
    }

    // -------------------------------------------------------------------------
    // makeConfirmWindow
    // -------------------------------------------------------------------------
    // Builds a Window hosting a ConfirmState with a prompt string.
    //
    // onConfirm — called when BUTTON_4 (Yes) is pressed, receives the Window.
    //             Typically calls Utilities::popWindow() and then acts on the
    //             result.
    // onCancel  — called when BUTTON_3 (No) is pressed.
    //             Defaults to Utilities::popWindow() if nullptr.
    //
    // Usage:
    //   auto win = makeConfirmWindow(
    //       "Delete all data?",
    //       [](Window &) { DataManager::eraseAll(); Utilities::popWindow(); },
    //       nullptr   // default cancel = pop window
    //   );
    //   Utilities::pushWindow(win);

    inline std::shared_ptr<Window>
    makeConfirmWindow(
        std::string                           prompt,
        std::function<void(Window &)>         onConfirm,
        std::function<void(Window &)>         onCancel = nullptr)
    {
        auto confirmState = std::make_shared<ConfirmState>();
        confirmState->setPrompt(prompt);

        auto win = std::make_shared<Window>();
        win->setInitialState(confirmState);

        // BUTTON_3 — No / cancel
        win->addInputCommand(InputID::BUTTON_3,
            [win, onCancel](const InputContext &)
            {
                if (onCancel)
                    onCancel(*win);
                else
                    Utilities::popWindow();
            });

        // BUTTON_4 — Yes / confirm
        win->addInputCommand(InputID::BUTTON_4,
            [win, onConfirm, confirmState](const InputContext &)
            {
                confirmState->confirm(true);
                if (onConfirm)
                    onConfirm(*win);
                else
                    Utilities::popWindow();
            });

        return win;
    }

    inline std::shared_ptr<SettingsWindow>
    makeSettingsWindow()
    {
        auto win = std::make_shared<SettingsWindow>();
        return win;
    }

    // -------------------------------------------------------------------------
    // initDefaultLayers
    // -------------------------------------------------------------------------
    // Registers ContentLayer and WindowLayer at their canonical IDs.
    // Call once at startup after Manager::init().
    //
    // Usage:
    //   DisplayModule::Manager::init(&display, 128, 32);
    //   DisplayModule::initDefaultLayers();

    inline void initDefaultLayers()
    {
        Utilities::registerLayer(
            LayerID::CONTENT,
            std::make_shared<ContentLayer>());

        Utilities::registerLayer(
            LayerID::WINDOW,
            std::make_shared<WindowLayer>());
    }

} // namespace DisplayModule
