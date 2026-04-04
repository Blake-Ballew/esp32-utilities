#pragma once

#include <string>
#include <cstdio>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // DiagnosticsState
    // -------------------------------------------------------------------------
    // Displays live system diagnostics: free heap (with unit scaling), heap
    // fragmentation, and FreeRTOS task stack high-water mark.
    //
    // Refreshes at 500 ms via refreshIntervalMs().
    //
    // Wiring example (Window):
    //   registerInput(InputID::BUTTON_3, "Back");
    //   addInputCommand(InputID::BUTTON_3, [](auto &) { Utilities::popWindow(); });

    class DiagnosticsState : public WindowState
    {
    public:
        static constexpr uint32_t REFRESH_RATE_MS = 500;

        DiagnosticsState()
        {
            bindInput(InputID::BUTTON_3, "Back");
            refreshIntervalMs = REFRESH_RATE_MS;
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &) override
        {
            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Tick — update diagnostics each cycle
        // ------------------------------------------------------------------

        void onTick() override
        {
            _rebuildDrawCommands();
        }

    private:
        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            // Heap free with unit scaling
            uint32_t heap = ESP.getFreeHeap();
            const char *units = "b";
            if (heap > 1024) { heap >>= 10; units = "Kb"; }
            if (heap > 1024) { heap >>= 10; units = "Mb"; }

            char line1[32];
            snprintf(line1, sizeof(line1), "Heap: %lu%s",
                     static_cast<unsigned long>(heap), units);

            // Fragmentation: maxContiguous / freeHeap
            char line2[32];
            if (ESP.getFreeHeap() > 0)
            {
                float frag = static_cast<float>(ESP.getMaxAllocHeap())
                             / static_cast<float>(ESP.getFreeHeap());
                snprintf(line2, sizeof(line2), "Frag: %.0f%%",
                         static_cast<double>(frag * 100.0f));
            }
            else
            {
                snprintf(line2, sizeof(line2), "Frag: N/A");
            }

            // Stack high-water mark of the calling task (display task)
            char line3[32];
            snprintf(line3, sizeof(line3), "Stack Min: %lu",
                     static_cast<unsigned long>(uxTaskGetStackHighWaterMark(NULL)));

            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(line1),
                TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 1 }
            ));
            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(line2),
                TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 2 }
            ));
            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(line3),
                TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 3 }
            ));
        }
    };

} // namespace DisplayModule
