#include <graphics/desktop.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::drivers;
using namespace crystalos::graphics;
using namespace crystalos::util;


Window::Window(uint32_t* parent)
: KeyboardEventHandler()
{
    Parent = parent;
    SizeX = 512;
    SizeY = 256;
    Selected = false;
}

Window::Window(uint32_t* parent, uint32_t LocationX, uint32_t LocationY)
: KeyboardEventHandler()
{
    Parent = parent;
    this->LocationX = LocationX;
    this->LocationY = LocationY;
    SizeX = 512;
    SizeY = 256;
    BackgroundR = 100;
    BackgroundG = 100;
    BackgroundB = 100;

    lastChildIndex = 0;
    currentChild = -1;
    Selected = false;
}

Window::~Window()
{
    
}

void Window::Draw()
{
    ((Desktop*)Parent)->FillRectangle(LocationX, LocationY, SizeX, SizeY, BackgroundR, BackgroundG, BackgroundB);
}

void Window::drawBorder()
{
    //Horizontal
    ((Desktop*)Parent)->DrawLine(LocationX, LocationY, LocationX + SizeX, LocationY, 2, 255, 0, 0);
    ((Desktop*)Parent)->DrawLine(LocationX, LocationY + SizeY - 2,LocationX + SizeX, LocationY + SizeY - 2, 2, 255, 0, 0);
    //Vertical
    ((Desktop*)Parent)->DrawLine(LocationX, LocationY, LocationX, LocationY + SizeY, 2, 255, 0, 0);
    ((Desktop*)Parent)->DrawLine(LocationX + SizeX - 2, LocationY, LocationX + SizeX - 2 ,LocationY + SizeY, 2, 255, 0, 0);
}

void Window::clearBorder()
{
    //Horizontal
    ((Desktop*)Parent)->DrawLine(LocationX, LocationY, LocationX + SizeX, LocationY, 2, BackgroundR, BackgroundG, BackgroundB);
    ((Desktop*)Parent)->DrawLine(LocationX, LocationY + SizeY - 2, LocationX + SizeX,LocationY + SizeY - 2, 2, BackgroundR, BackgroundG, BackgroundB);
    //Vertical
    ((Desktop*)Parent)->DrawLine(LocationX, LocationY, LocationX, LocationY + SizeY, 2, BackgroundR, BackgroundG, BackgroundB);
    ((Desktop*)Parent)->DrawLine(LocationX + SizeX - 2, LocationY, LocationX + SizeX - 2 ,LocationY + SizeY, 2, BackgroundR, BackgroundG, BackgroundB);
}

void Window::OnKeyDown(char c)
{

}

void Window::OnKeyUp(char c)
{
    
}