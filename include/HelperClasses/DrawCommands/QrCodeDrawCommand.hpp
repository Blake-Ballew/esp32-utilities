#pragma once

#include "DrawCommand.hpp"
#include "Interfaces/DisplayTypes.hpp"
#include <pgmspace.h>

namespace DisplayModule
{

struct QrFormat
{
    TextAlignH hAlign  = TextAlignH::CENTER;
    TextAlignV vAlign  = TextAlignV::CENTER;
    int16_t    offsetX = 0;
    int16_t    offsetY = 0;
    uint8_t    line    = 1;

    QrFormat() = default;
    QrFormat(TextAlignH h, TextAlignV v, int16_t ox = 0, int16_t oy = 0, uint8_t ln = 1)
        : hAlign(h), vAlign(v), offsetX(ox), offsetY(oy), line(ln) {}
};

class QrCodeDrawCommand : public DrawCommand
{
public:
    QrCodeDrawCommand(const uint8_t *bitmap, int16_t modules, int16_t scale = 1, QrFormat fmt = {})
        : _bitmap(bitmap), _modules(modules), _scale(scale), _fmt(fmt) {}

    void draw(DrawContext &ctx) override
    {
        int16_t stride = (_modules + 7) / 8;
        int16_t size   = _modules * _scale;

        int16_t x = resolveX(ctx, size);
        int16_t y = resolveY(ctx, size);

        for (int16_t row = 0; row < _modules; row++)
        {
            for (int16_t col = 0; col < _modules; col++)
            {
                uint8_t byte = pgm_read_byte(&_bitmap[row * stride + col / 8]);
                if (byte & (0x80 >> (col % 8)))
                {
                    ctx.display->fillRect(
                        x + col * _scale,
                        y + row * _scale,
                        _scale, _scale,
                        DrawColorPrimary());
                }
            }
        }
    }

private:
    const uint8_t *_bitmap;
    int16_t        _modules;
    int16_t        _scale;
    QrFormat       _fmt;

    int16_t resolveX(const DrawContext &ctx, int16_t size) const
    {
        switch (_fmt.hAlign)
        {
            case TextAlignH::LEFT:
                return _fmt.offsetX;
            case TextAlignH::RIGHT:
                return static_cast<int16_t>(ctx.width - size - _fmt.offsetX);
            default: // CENTER
                return static_cast<int16_t>((ctx.width - size) / 2 + _fmt.offsetX);
        }
    }

    int16_t resolveY(const DrawContext &ctx, int16_t size) const
    {
        constexpr int16_t LINE_H = 8;
        switch (_fmt.vAlign)
        {
            case TextAlignV::TOP:
            case TextAlignV::CONTENT_TOP:
                return _fmt.offsetY;
            case TextAlignV::BOTTOM:
            case TextAlignV::CONTENT_BOTTOM:
                return static_cast<int16_t>(ctx.height - size - _fmt.offsetY);
            case TextAlignV::LINE:
                return static_cast<int16_t>((_fmt.line > 0 ? _fmt.line - 1 : 0) * LINE_H + _fmt.offsetY);
            default: // CENTER
                return static_cast<int16_t>((ctx.height - size) / 2 + _fmt.offsetY);
        }
    }
};

} // namespace DisplayModule
