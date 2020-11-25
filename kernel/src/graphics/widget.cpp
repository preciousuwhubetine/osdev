#include <graphics/desktop.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::drivers;
using namespace crystalos::graphics;
using namespace crystalos::util;

Widget::Widget(uint32_t* parent, WidgetType type, int xpos, int ypos)
: KeyboardEventHandler(),
  EventReceiver()
{
    Selected = false;
    Type = type;
    Parent = parent; 
    // ((Desktop*)Parent)->AddChild(this);

    if (type == Icon) //Icon
    {
        
    }
    else
    {
        XPos = xpos;
        YPos = ypos;
    }
    BackgroundB = 0xff;
    BackgroundG = 0xff;
    BackgroundR = 0xff;
}

Widget::~Widget()
{

}

void Widget::Draw()
{
    
}

void Widget::drawBorder()
{
    //Horizontal
    ((Desktop*)Parent)->DrawLine(XPos, YPos, XPos + Width, YPos, 1, 255, 0, 0);
    ((Desktop*)Parent)->DrawLine(XPos, YPos + Height - 1, XPos + Width, YPos + Height - 1, 1, 255, 0, 0);
    //Vertical
    ((Desktop*)Parent)->DrawLine(XPos, YPos, XPos, YPos + Height, 1, 255, 0, 0);
    ((Desktop*)Parent)->DrawLine(XPos + Width - 1, YPos, XPos + Width - 1 ,YPos + Height, 1, 255, 0, 0);
}

void Widget::clearBorder()
{
    //Horizontal
    ((Desktop*)Parent)->DrawLine(XPos, YPos, XPos + Width, YPos, 1, BackgroundR, BackgroundG, BackgroundB);
    ((Desktop*)Parent)->DrawLine(XPos, YPos + Height - 1, XPos + Width, YPos + Height - 1, 1, BackgroundR, BackgroundG, BackgroundB);
    //Vertical
    ((Desktop*)Parent)->DrawLine(XPos, YPos, XPos, YPos + Height, 1, BackgroundR, BackgroundG, BackgroundB);
    ((Desktop*)Parent)->DrawLine(XPos + Width - 1, YPos, XPos + Width - 1 ,YPos + Height, 1, BackgroundR, BackgroundG, BackgroundB);
}

void Widget::OnKeyDown(char c)
{
   
}

void Widget::OnKeyUp(char c)
{

}