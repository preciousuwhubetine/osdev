#ifndef CRYSTALOS__GRAPHICS__WINDOW_H
#define CRYSTALOS__GRAPHICS__WINDOW_H

#include <drivers/keyboard.h>
#include <graphics/widget.h>

namespace crystalos
{
    namespace graphics
    {
        class Window : public drivers::KeyboardEventHandler
        {
            protected:
                common::uint32_t* Parent;
                common::uint8_t BackgroundR;
                common::uint8_t BackgroundG;
                common::uint8_t BackgroundB;
                int lastChildIndex;
                int currentChild;

            public:
                bool Selected;
                
                common::uint32_t SizeX;
                common::uint32_t SizeY;
                common::uint32_t LocationX;
                common::uint32_t LocationY;
                Widget* Children[256];
                Window(common::uint32_t* parent);
                Window(common::uint32_t* parent, common::uint32_t LocationX, common::uint32_t LocationY);
                virtual void clearBorder();
                virtual void drawBorder();
                virtual void Draw();
                virtual void OnKeyDown(char);
                virtual void OnKeyUp(char);
                ~Window();
        };

    }
}


#endif