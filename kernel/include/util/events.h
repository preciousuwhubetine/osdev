#ifndef CRYSTALOS__UTIL__EVENTS_H
#define CRYSTALOS__UTIL__EVENTS_H

#include <common/types.h>

namespace crystalos
{
    namespace util
    {
        enum EVENTS
        {
            OnKeyDown = 0, OnKeyUp = 1, OnMouseDown = 2, OnMouseUp = 3, OnMouseMove = 4, 
            SelectedIndexChanged = 5, OnFocus = 6
        };

        class EventArgs
        {
            public:
                common::uint32_t* sender;
                EventArgs();
                ~EventArgs();
                bool Suppress;
        };

        class KeyBoardEventArgs: public EventArgs
        {
            public:
                char Key;
                KeyBoardEventArgs();
                ~KeyBoardEventArgs();
        };

        class EventReceiver
        {
            public:
                void (*events[256])(EventArgs*);
                bool isEventAssigned(EVENTS);
                EventReceiver();
                ~EventReceiver();
        };

        void AddHandler(EventReceiver*, EVENTS, void(*)(EventArgs*));

    }
}
#endif