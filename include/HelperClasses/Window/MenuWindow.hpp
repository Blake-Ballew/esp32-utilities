#pragma once

#include <string>
#include <functional>
#include "Window.hpp"
#include "States/MenuState.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // MenuWindow
    // -------------------------------------------------------------------------
    // Ready-to-use window that hosts a MenuState and wires the standard
    // input set automatically.
    //
    // Default input wiring:
    //   ENC_UP   (InputID 4) — scroll up
    //   ENC_DOWN (InputID 5) — scroll down
    //   BUTTON_4 (InputID 3) — select current item  label: "Select"
    //   BUTTON_3 (InputID 2) — back / pop window    label: "Back"
    //
    // The BUTTON_3 back action pops the window from the Utilities stack.
    // Override or suppress it by calling clearInput(InputID::BUTTON_3) after
    // construction.
    //
    // Adding menu items:
    //   auto win = std::make_shared<MenuWindow>();
    //   win->addItem("Calibrate", calibrateState);
    //   win->addItem("Debug",     debugState);
    //   win->addItem("Settings",  nullptr, [](Window &) {
    //       // custom action, e.g. push a new window
    //       Utilities::pushWindow(std::make_shared<SettingsWindow>());
    //   });
    //   Utilities::pushWindow(win);

    class MenuWindow : public Window
    {
    public:
        MenuWindow()
        {
            _menuState = std::make_shared<MenuState>();
            
            // BUTTON_4 — select current item
            registerInput(InputID::BUTTON_4, "Select");
            addInputCommand(InputID::BUTTON_4,
                [this](const InputContext &)
                {
                    _menuState->selectCurrent();
                });

            // BUTTON_3 — pop this window
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &)
                {
                    Utilities::popWindow();
                });

            setInitialState(_menuState);
        }

        // ------------------------------------------------------------------
        // Item management — delegates to the inner MenuState
        // ------------------------------------------------------------------

        void addItem(std::string label,
                     std::function<void()> cb = nullptr)
        {
            _menuState->addItem(std::move(label), std::move(cb));
        }

        void addItem(MenuItem item)
        {
            _menuState->addItem(std::move(item));
        }

        void clearItems() { _menuState->clearItems(); }

        size_t itemCount()    const { return _menuState->itemCount(); }
        size_t currentIndex() const { return _menuState->currentIndex(); }

        std::shared_ptr<MenuState> menuState() { return _menuState; }

    private:
        std::shared_ptr<MenuState> _menuState;
    };

} // namespace DisplayModule
