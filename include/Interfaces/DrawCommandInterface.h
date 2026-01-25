#include "Adafruit_GFX.h"

namespace UxModule
{
    class DrawContext
    {
    public:
        Adafruit_GFX* display;
        uint8_t width;
        uint8_t height;
        
        DrawContext(Adafruit_GFX* disp, uint8_t w, uint8_t h) 
            : display(disp), width(w), height(h) {}
        
        // Helper to convert percentage to pixels
        int16_t toPixelX(float percent) const {
            return (int16_t)(width * percent);
        }
        
        int16_t toPixelY(float percent) const {
            return (int16_t)(height * percent);
        }
    };

    class DrawCommandInterface
    {
    public:
        virtual void Draw(DrawContext &context) = 0;
    };
}
