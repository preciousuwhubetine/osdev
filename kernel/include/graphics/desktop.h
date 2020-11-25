#ifndef CRYSTALOS__GRAPHICS__DESKTOP_H
#define CRYSTALOS__GRAPHICS__DESKTOP_H

#include <drivers/keyboard.h>
#include <graphics/vesa.h>
#include <graphics/window.h>
#include <graphics/widget.h>
#include <util/number.h>
#include <util/string.h>
#include <util/events.h>
#include <util/images/bitmap.h>

namespace crystalos
{
    namespace graphics
    {
        class Desktop : public drivers::KeyboardEventHandler
        {
            protected:
                int lastWindowIndex;    
                int lastWidgetIndex;
                int num_files;

			    bool PassToWindow;
                drivers::File* files;
                
                common::uint8_t* FontBuffer;

                int currentWindowIndex;
                
                void OnKeyDown(char);
                void OnKeyUp(char);

            public:
                VESAModeInfo* currentMode;
                common::uint32_t Width;
                common::uint32_t Height;

                static Desktop* OSDesktop;

                int currentWidgetIndex;
                Widget* Children[256];
                Window* Windows[256];
                common::uint32_t* screenMemory;
                common::uint32_t* doubleBuffer;

                void AddChild(Widget* handle);
                void AddChild(Window* handle);

                void PutPixel(common::uint32_t x ,common::uint32_t y, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void DrawBackground(common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void SetBackgroundColor(common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void FillRectangle(common::uint32_t x, common::uint32_t y, common::uint32_t w, common::uint32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void DrawBorder(Widget* handle);
                void DrawBorder(Window* handle);
                void ClearBorder(Widget* handle);
                void ClearBorder(Window* handle);
                void DrawTransparentChar16(char c, int x, int y, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void DrawText(char* string, int x, int y, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void DrawLine(int x1, int y1, int x2, int y2, int thickness, common::uint8_t r, common::uint8_t g, common::uint8_t b);
                void Draw();
                void DrawImage(util::images::Bitmap* bmp, int locX, int locY, int zoom);
                void DrawImage(util::images::Bitmap* bmp, int locX, int locY, int contWidth, int contHeight);

                Desktop(VESAModeInfo* screen);
                ~Desktop();

        };

    }
}


#endif