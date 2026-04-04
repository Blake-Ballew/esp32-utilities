#pragma once

// LayerInterface.hpp
// Sits below Utilities in the dependency graph — no Utilities include here.
//
// Dependency chain:
//   DisplayTypes.hpp
//       ↓
//   LayerInterface.hpp    ←  Utilities depends on this
//       ↓
//   DisplayUtilities.hpp
//       ↓
//   ContentLayer.hpp / WindowLayer.hpp  ←  concrete implementations

#include "DisplayTypes.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // LayerInterface
    // -------------------------------------------------------------------------
    // Base interface for all render layers. Layers are registered in Utilities
    // in a map<uint8_t, shared_ptr<LayerInterface>> keyed by LayerID, and
    // rendered in ascending ID order each frame.
    //
    // Built-in layer IDs (see LayerID namespace in DisplayTypes.hpp):
    //   CONTENT = 0  — state draw commands (lowest priority, drawn first)
    //   WINDOW  = 1  — window draw commands + input labels (drawn on top)
    //   USER_BASE = 8+ — app-defined layers (HUD, edge compass, etc.)
    //
    // Layers can be disabled at runtime without being removed from the pipeline.

    class LayerInterface
    {
    public:
        virtual ~LayerInterface() = default;

        // Called once per frame by Utilities::render() if enabled
        virtual void draw(DrawContext &ctx) = 0;

        bool isEnabled() const { return _enabled; }
        void setEnabled(bool enabled) { _enabled = enabled; }

    protected:
        bool _enabled = true;
    };

} // namespace DisplayModule
