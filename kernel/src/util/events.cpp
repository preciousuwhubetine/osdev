#include <util/events.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;

EventReceiver::EventReceiver()
{
    for (int i = 0; i < 256; i++)
        events[i] = 0;
}

EventReceiver::~EventReceiver()
{}

void util::AddHandler(EventReceiver* receiver, EVENTS event, void(*handler)(EventArgs*))
{
    receiver->events[event] = handler;
}

bool EventReceiver::isEventAssigned(EVENTS event)
{
    if (events[event] == 0) return false;
    return true;
}

EventArgs::EventArgs()
{
    sender = 0;
    Suppress = false;
}

EventArgs::~EventArgs()
{

}

KeyBoardEventArgs::KeyBoardEventArgs()
{}

KeyBoardEventArgs::~KeyBoardEventArgs()
{}