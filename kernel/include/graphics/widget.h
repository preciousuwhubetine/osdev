#ifndef CRYSTALOS__GRAPHICS__WIDGET_H
#define CRYSTALOS__GRAPHICS__WIDGET_H

#include <drivers/keyboard.h>
#include <drivers/filesystem.h>
#include <util/system.h>
#include <util/events.h>

namespace crystalos
{
    namespace graphics
    {
        enum WidgetType
        {
            LabelWidget = 1,
            Icon = 2,
            TextBoxWidget = 3,
            ListBoxWidget = 4,
            Other = 5
        };

        class Widget : public drivers::KeyboardEventHandler, public util::EventReceiver
        {
            protected:

                common::uint8_t BackgroundR;
                common::uint8_t BackgroundG;
                common::uint8_t BackgroundB;

                common::uint8_t ForegroundR;
                common::uint8_t ForegroundG;
                common::uint8_t ForegroundB;

            public:
                bool Selected;
                void OnKeyDown(char);
                void OnKeyUp(char);
                common::uint32_t* Parent;
                int handle;
                WidgetType Type;
                common::uint32_t Width;
                common::uint32_t Height;
                common::uint32_t XPos;
                common::uint32_t YPos;

                virtual void Draw();
                void drawBorder();
                void clearBorder();
                Widget(common::uint32_t* parent, WidgetType type, int xpos, int ypos);
                ~Widget();
        };
    }
}


#endif