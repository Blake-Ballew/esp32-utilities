#pragma once

#include <string>
#include "DrawCommand.hpp"
#include "DisplayUtilities.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // TextDrawCommand
    // -------------------------------------------------------------------------
    // Renders a single string using a TextFormat descriptor.
    //
    // Coordinate resolution rules (all based on Adafruit default 6×8 font):
    //
    //   hAlign
    //     LEFT   — x = distanceFrom * 6  (character columns from the left edge)
    //     CENTER — x = centre of display ± distanceFrom pixels
    //     RIGHT  — x = right-aligned, distanceFrom character columns from the right
    //
    //   vAlign
    //     TOP            — y = 0
    //     CENTER         — y = (height/2) - 4
    //     BOTTOM         — y = height - 8
    //     CONTENT_TOP    — alias for TOP (states may redefine via sub-classing)
    //     CONTENT_BOTTOM — alias for BOTTOM
    //     LINE           — y = (line - 1) * 8  (TextFormat::line is 1-based)
    //
    //   distanceFrom
    //     For LEFT/RIGHT: column offset in characters.
    //     For CENTER: pixel offset (positive = right, negative = left).

    class TextDrawCommand : public DrawCommand
    {
    public:
        std::string text;
        TextFormat  format;

        TextDrawCommand() = default;

        explicit TextDrawCommand(std::string t, TextFormat fmt = {})
            : text(std::move(t)), format(fmt) {}

        void draw(DrawContext &ctx) override
        {
            if (!ctx.display || text.empty()) return;

            const uint16_t x = resolveX(ctx, text.size(), format);
            const uint16_t y = resolveY(ctx, format);

            ESP_LOGV(TAG, "Placing text cursor for %s at %d, %d", text.c_str(), x, y);

            ctx.display->setTextColor(DrawColorPrimary());
            ctx.display->setCursor(static_cast<int16_t>(x),
                                   static_cast<int16_t>(y));
            ctx.display->print(text.c_str());
        }

        // ------------------------------------------------------------------
        // Static helpers — reusable by other draw commands
        // ------------------------------------------------------------------

        static uint16_t resolveX(const DrawContext &ctx,
                                  size_t charCount,
                                  const TextFormat &fmt)
        {
            constexpr uint16_t W = 6;
            switch (fmt.hAlign)
            {
                case TextAlignH::LEFT:
                    return static_cast<uint16_t>(fmt.distanceFrom * W);

                case TextAlignH::RIGHT:
                    return static_cast<uint16_t>(
                        ctx.width
                        - (static_cast<uint16_t>(charCount) * W)
                        - (fmt.distanceFrom * W));

                default: // CENTER
                    return static_cast<uint16_t>(
                        (ctx.width / 2)
                        - (static_cast<uint16_t>(charCount) * W / 2)
                        + (fmt.distanceFrom * W));
            }
        }

        static uint16_t resolveY(const DrawContext &ctx, const TextFormat &fmt)
        {
            constexpr uint16_t H = 8;
            switch (fmt.vAlign)
            {
                case TextAlignV::TOP:
                case TextAlignV::CONTENT_TOP:
                    return 0;

                case TextAlignV::BOTTOM:
                case TextAlignV::CONTENT_BOTTOM:
                    return static_cast<uint16_t>(ctx.height - H);

                case TextAlignV::LINE:
                    return static_cast<uint16_t>((fmt.line > 0 ? fmt.line - 1 : 0) * H);

                default: // CENTER
                    return static_cast<uint16_t>((ctx.height / 2) - (H / 2));
            }
        }

        static std::shared_ptr<TextDrawCommand> createCenteredMessage(const std::string &msg)
        {
            return std::make_shared<TextDrawCommand>(msg, TextFormat{TextAlignH::CENTER, TextAlignV::CENTER});
        }
    };

} // namespace DisplayModule
