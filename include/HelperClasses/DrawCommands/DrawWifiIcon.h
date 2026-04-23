#include "DrawCommand.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // DrawWifiIcon
    // -------------------------------------------------------------------------
    // Draws a 12×8 px Wi-Fi signal icon at (x, y).
    //
    // The icon is a series of concentric arcs with a dot in the center:
    //
    //   .       (dot)
    //   ┌───┐   (small arc)
    //  ┌─────┐  (medium arc)
    // ┌───────┐ (large arc)
{
    class DrawWifiIcon : public DrawCommand
    {
        public:
        int x = 0;
        int y = 0;

        void draw(DrawContext &context) override
        {
            auto display = context.display;
            display->fillRect(x, y, context.height, context.width, BLACK);

            //id: 0 pixel 95 
            display->drawPixel(x + 0, y + 7, DrawColorPrimary());
            //id: 1 pixel 96 
            display->drawPixel(x + 11, y + 7, DrawColorPrimary());
            //id: 2 pixel 97 
            display->drawPixel(x + 11, y + 6, DrawColorPrimary());
            //id: 3 pixel 98 
            display->drawPixel(x + 11, y + 5, DrawColorPrimary());
            //id: 4 pixel 99 
            display->drawPixel(x + 11, y + 4, DrawColorPrimary());
            //id: 5 pixel 100 
            display->drawPixel(x + 0, y + 6, DrawColorPrimary());
            //id: 6 pixel 101 
            display->drawPixel(x + 0, y + 5, DrawColorPrimary());
            //id: 7 pixel 102 
            display->drawPixel(x + 0, y + 4, DrawColorPrimary());
            //id: 8 pixel 111 
            display->drawPixel(x + 7, y + 0, DrawColorPrimary());
            display->drawPixel(x + 6, y + 0, DrawColorPrimary());
            display->drawPixel(x + 5, y + 0, DrawColorPrimary());
            display->drawPixel(x + 4, y + 0, DrawColorPrimary());
            //id: 9 pixel 130 
            display->drawPixel(x + 2, y + 7, DrawColorPrimary());
            //id: 10 pixel 131 
            display->drawPixel(x + 2, y + 6, DrawColorPrimary());
            //id: 11 pixel 132 
            display->drawPixel(x + 2, y + 5, DrawColorPrimary());
            //id: 12 pixel 138 
            display->drawPixel(x + 9, y + 7, DrawColorPrimary());
            //id: 13 pixel 140 
            display->drawPixel(x + 9, y + 6, DrawColorPrimary());
            //id: 14 pixel 141 
            display->drawPixel(x + 9, y + 5, DrawColorPrimary());
            //id: 15 pixel 144 
            display->drawPixel(x + 5, y + 7, DrawColorPrimary());
            display->drawPixel(x + 6, y + 7, DrawColorPrimary());
            display->drawPixel(x + 6, y + 6, DrawColorPrimary());
            display->drawPixel(x + 5, y + 6, DrawColorPrimary());
            //id: 16 pixel 145 
            display->drawPixel(x + 0, y + 3, DrawColorPrimary());
            //id: 17 pixel 146 
            display->drawPixel(x + 1, y + 2, DrawColorPrimary());
            //id: 18 pixel 147 
            display->drawPixel(x + 2, y + 1, DrawColorPrimary());
            //id: 19 pixel 148 
            display->drawPixel(x + 3, y + 0, DrawColorPrimary());
            //id: 20 pixel 149 
            display->drawPixel(x + 11, y + 3, DrawColorPrimary());
            //id: 21 pixel 150 
            display->drawPixel(x + 10, y + 2, DrawColorPrimary());
            //id: 22 pixel 151 
            display->drawPixel(x + 9, y + 1, DrawColorPrimary());
            //id: 23 pixel 152 
            display->drawPixel(x + 8, y + 0, DrawColorPrimary());
            //id: 24 pixel 153 
            display->drawPixel(x + 2, y + 4, DrawColorPrimary());
            //id: 25 pixel 154 
            display->drawPixel(x + 3, y + 3, DrawColorPrimary());
            //id: 26 pixel 155 
            display->drawPixel(x + 4, y + 2, DrawColorPrimary());
            //id: 27 pixel 156 
            display->drawPixel(x + 5, y + 2, DrawColorPrimary());
            //id: 28 pixel 157 
            display->drawPixel(x + 6, y + 2, DrawColorPrimary());
            //id: 29 pixel 158 
            display->drawPixel(x + 7, y + 2, DrawColorPrimary());
            //id: 30 pixel 159 
            display->drawPixel(x + 7, y + 2, DrawColorPrimary());
            //id: 31 pixel 160 
            display->drawPixel(x + 8, y + 3, DrawColorPrimary());
            //id: 32 pixel 161 
            display->drawPixel(x + 9, y + 4, DrawColorPrimary());
        }
    };
}