#include "DrawCommandInterface.h"
#include "Display_Utils.h"

namespace UxModule
{
    class DrawWifiIcon : public DrawCommandInterface
    {
        public:
        int x = 0;
        int y = 0;

        void Draw(DrawContext &context) override
        {
            auto display = context.display;
            display->fillRect(x, y, context.height, context.width, BLACK);

            //id: 0 pixel 95 
            display->drawPixel(x + 0, y + 7, SSD1306_WHITE);
            //id: 1 pixel 96 
            display->drawPixel(x + 11, y + 7, SSD1306_WHITE);
            //id: 2 pixel 97 
            display->drawPixel(x + 11, y + 6, SSD1306_WHITE);
            //id: 3 pixel 98 
            display->drawPixel(x + 11, y + 5, SSD1306_WHITE);
            //id: 4 pixel 99 
            display->drawPixel(x + 11, y + 4, SSD1306_WHITE);
            //id: 5 pixel 100 
            display->drawPixel(x + 0, y + 6, SSD1306_WHITE);
            //id: 6 pixel 101 
            display->drawPixel(x + 0, y + 5, SSD1306_WHITE);
            //id: 7 pixel 102 
            display->drawPixel(x + 0, y + 4, SSD1306_WHITE);
            //id: 8 pixel 111 
            display->drawPixel(x + 7, y + 0, SSD1306_WHITE);
            display->drawPixel(x + 6, y + 0, SSD1306_WHITE);
            display->drawPixel(x + 5, y + 0, SSD1306_WHITE);
            display->drawPixel(x + 4, y + 0, SSD1306_WHITE);
            //id: 9 pixel 130 
            display->drawPixel(x + 2, y + 7, SSD1306_WHITE);
            //id: 10 pixel 131 
            display->drawPixel(x + 2, y + 6, SSD1306_WHITE);
            //id: 11 pixel 132 
            display->drawPixel(x + 2, y + 5, SSD1306_WHITE);
            //id: 12 pixel 138 
            display->drawPixel(x + 9, y + 7, SSD1306_WHITE);
            //id: 13 pixel 140 
            display->drawPixel(x + 9, y + 6, SSD1306_WHITE);
            //id: 14 pixel 141 
            display->drawPixel(x + 9, y + 5, SSD1306_WHITE);
            //id: 15 pixel 144 
            display->drawPixel(x + 5, y + 7, SSD1306_WHITE);
            display->drawPixel(x + 6, y + 7, SSD1306_WHITE);
            display->drawPixel(x + 6, y + 6, SSD1306_WHITE);
            display->drawPixel(x + 5, y + 6, SSD1306_WHITE);
            //id: 16 pixel 145 
            display->drawPixel(x + 0, y + 3, SSD1306_WHITE);
            //id: 17 pixel 146 
            display->drawPixel(x + 1, y + 2, SSD1306_WHITE);
            //id: 18 pixel 147 
            display->drawPixel(x + 2, y + 1, SSD1306_WHITE);
            //id: 19 pixel 148 
            display->drawPixel(x + 3, y + 0, SSD1306_WHITE);
            //id: 20 pixel 149 
            display->drawPixel(x + 11, y + 3, SSD1306_WHITE);
            //id: 21 pixel 150 
            display->drawPixel(x + 10, y + 2, SSD1306_WHITE);
            //id: 22 pixel 151 
            display->drawPixel(x + 9, y + 1, SSD1306_WHITE);
            //id: 23 pixel 152 
            display->drawPixel(x + 8, y + 0, SSD1306_WHITE);
            //id: 24 pixel 153 
            display->drawPixel(x + 2, y + 4, SSD1306_WHITE);
            //id: 25 pixel 154 
            display->drawPixel(x + 3, y + 3, SSD1306_WHITE);
            //id: 26 pixel 155 
            display->drawPixel(x + 4, y + 2, SSD1306_WHITE);
            //id: 27 pixel 156 
            display->drawPixel(x + 5, y + 2, SSD1306_WHITE);
            //id: 28 pixel 157 
            display->drawPixel(x + 6, y + 2, SSD1306_WHITE);
            //id: 29 pixel 158 
            display->drawPixel(x + 7, y + 2, SSD1306_WHITE);
            //id: 30 pixel 159 
            display->drawPixel(x + 7, y + 2, SSD1306_WHITE);
            //id: 31 pixel 160 
            display->drawPixel(x + 8, y + 3, SSD1306_WHITE);
            //id: 32 pixel 161 
            display->drawPixel(x + 9, y + 4, SSD1306_WHITE);
        }
    };
}