#include <drivers/keyboard.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::IO;
using namespace crystalos::drivers;
using namespace crystalos::util;

KeyboardDriver* KeyboardDriver::Main = 0;

KeyboardEventHandler::KeyboardEventHandler()
{

}

void KeyboardEventHandler::OnKeyDown(char)
{

}

void KeyboardEventHandler::OnKeyUp(char)
{

}

KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler* kbhandler)
: Driver(),
  InterruptHandler(manager, 0x21),
  commandPort(0x64),
  dataPort(0x60)
{
    kb_handler = kbhandler;
    kb_handler->base = this;
    if (Main == 0) Main = this;
}

void KeyboardDriver::Activate()
{
    while(commandPort.Read() & 0x1)
	{
		dataPort.Read();
	}
	commandPort.Write(0xAE); //Start sending interrupts
	commandPort.Write(0x20); // get current state
	uint8_t status = (dataPort.Read() |  1) & ~0x10;
	commandPort.Write(0x60); //set state
	dataPort.Write(status);
	
	dataPort.Write(0xF4);

}

uint32_t KeyboardDriver::HandleInterrupt(common::uint32_t esp)
{
    uint8_t key = dataPort.Read();
    if (kb_handler == 0)
    {
        return esp;
    }

    static bool shift = false;

    switch(key)
    {
        case 0xFA: break;
        case 0x02: if (!shift) kb_handler->OnKeyDown('1'); else kb_handler->OnKeyDown('!'); break;
        case 0x03: if (!shift) kb_handler->OnKeyDown('2'); else kb_handler->OnKeyDown('@'); break;
        case 0x04: if (!shift) kb_handler->OnKeyDown('3'); else kb_handler->OnKeyDown('#'); break;
        case 0x05: if (!shift) kb_handler->OnKeyDown('4'); else kb_handler->OnKeyDown('$'); break;
        case 0x06: if (!shift) kb_handler->OnKeyDown('5'); else kb_handler->OnKeyDown('%'); break;
        case 0x07: if (!shift) kb_handler->OnKeyDown('6'); else kb_handler->OnKeyDown('^'); break;
        case 0x08: if (!shift) kb_handler->OnKeyDown('7'); else kb_handler->OnKeyDown('&'); break;
        case 0x09: if (!shift) kb_handler->OnKeyDown('8'); else kb_handler->OnKeyDown('*'); break;
        case 0x0A: if (!shift) kb_handler->OnKeyDown('9'); else kb_handler->OnKeyDown('('); break;
        case 0x0B: if (!shift) kb_handler->OnKeyDown('0'); else kb_handler->OnKeyDown(')'); break;
        case 0x0F: if (!shift) kb_handler->OnKeyDown('\t'); else kb_handler->OnKeyDown(0xFF); break; //Tab
        case 0x10: if (!shift) kb_handler->OnKeyDown('q'); else kb_handler->OnKeyDown('Q'); break;
        case 0x11: if (!shift) kb_handler->OnKeyDown('w'); else kb_handler->OnKeyDown('W'); break;
        case 0x12: if (!shift) kb_handler->OnKeyDown('e'); else kb_handler->OnKeyDown('E'); break;
        case 0x13: if (!shift) kb_handler->OnKeyDown('r'); else kb_handler->OnKeyDown('R'); break;
        case 0x14: if (!shift) kb_handler->OnKeyDown('t'); else kb_handler->OnKeyDown('T'); break;
        case 0x15: if (!shift) kb_handler->OnKeyDown('y'); else kb_handler->OnKeyDown('Y'); break;
        case 0x16: if (!shift) kb_handler->OnKeyDown('u'); else kb_handler->OnKeyDown('U'); break;
        case 0x17: if (!shift) kb_handler->OnKeyDown('i'); else kb_handler->OnKeyDown('I'); break;
        case 0x18: if (!shift) kb_handler->OnKeyDown('o'); else kb_handler->OnKeyDown('O'); break;
        case 0x19: if (!shift) kb_handler->OnKeyDown('p'); else kb_handler->OnKeyDown('P'); break;
        case 0x1E: if (!shift) kb_handler->OnKeyDown('a'); else kb_handler->OnKeyDown('A'); break;
        case 0x1F: if (!shift) kb_handler->OnKeyDown('s'); else kb_handler->OnKeyDown('S'); break;
        case 0x20: if (!shift) kb_handler->OnKeyDown('d'); else kb_handler->OnKeyDown('D'); break;
        case 0x21: if (!shift) kb_handler->OnKeyDown('f'); else kb_handler->OnKeyDown('F'); break;
        case 0x22: if (!shift) kb_handler->OnKeyDown('g'); else kb_handler->OnKeyDown('G'); break;
        case 0x23: if (!shift) kb_handler->OnKeyDown('h'); else kb_handler->OnKeyDown('H'); break;
        case 0x24: if (!shift) kb_handler->OnKeyDown('j'); else kb_handler->OnKeyDown('J'); break;
        case 0x25: if (!shift) kb_handler->OnKeyDown('k'); else kb_handler->OnKeyDown('K'); break;
        case 0x26: if (!shift) kb_handler->OnKeyDown('l'); else kb_handler->OnKeyDown('L'); break;
        case 0x27: if (!shift) kb_handler->OnKeyDown(';'); else kb_handler->OnKeyDown(':'); break;
        case 0x28: if (!shift) kb_handler->OnKeyDown('\''); else kb_handler->OnKeyDown('\"'); break;
        case 0x2C: if (!shift) kb_handler->OnKeyDown('z'); else kb_handler->OnKeyDown('Z'); break;
        case 0x2D: if (!shift) kb_handler->OnKeyDown('x'); else kb_handler->OnKeyDown('X'); break;
        case 0x2E: if (!shift) kb_handler->OnKeyDown('c'); else kb_handler->OnKeyDown('C'); break;
        case 0x2F: if (!shift) kb_handler->OnKeyDown('v'); else kb_handler->OnKeyDown('V'); break;
        case 0x30: if (!shift) kb_handler->OnKeyDown('b'); else kb_handler->OnKeyDown('B'); break;
        case 0x31: if (!shift) kb_handler->OnKeyDown('n'); else kb_handler->OnKeyDown('N'); break;
        case 0x32: if (!shift) kb_handler->OnKeyDown('m'); else kb_handler->OnKeyDown('M'); break;
        case 0x33: if (!shift) kb_handler->OnKeyDown(','); else kb_handler->OnKeyDown('<'); break;
        case 0x34: if (!shift) kb_handler->OnKeyDown('.'); else kb_handler->OnKeyDown('>'); break;
        case 0x35: if (!shift) kb_handler->OnKeyDown('/'); else kb_handler->OnKeyDown('?'); break;
        
        case 0x39: kb_handler->OnKeyDown(' '); break;
        case 0x1c: kb_handler->OnKeyDown('\n'); break;
        case 0x0E: kb_handler->OnKeyDown('\b'); break;

        case 0x82: if (!shift) kb_handler->OnKeyUp('1'); else kb_handler->OnKeyUp('!'); break;
        case 0x83: if (!shift) kb_handler->OnKeyUp('2'); else kb_handler->OnKeyUp('@'); break;
        case 0x84: if (!shift) kb_handler->OnKeyUp('3'); else kb_handler->OnKeyUp('#'); break;
        case 0x85: if (!shift) kb_handler->OnKeyUp('4'); else kb_handler->OnKeyUp('$'); break;
        case 0x86: if (!shift) kb_handler->OnKeyUp('5'); else kb_handler->OnKeyUp('%'); break;
        case 0x87: if (!shift) kb_handler->OnKeyUp('6'); else kb_handler->OnKeyUp('^'); break;
        case 0x88: if (!shift) kb_handler->OnKeyUp('7'); else kb_handler->OnKeyUp('&'); break;
        case 0x89: if (!shift) kb_handler->OnKeyUp('8'); else kb_handler->OnKeyUp('*'); break;
        case 0x8A: if (!shift) kb_handler->OnKeyUp('9'); else kb_handler->OnKeyUp('('); break;
        case 0x8B: if (!shift) kb_handler->OnKeyUp('0'); else kb_handler->OnKeyUp(')'); break;
        case 0x8F: if (!shift) kb_handler->OnKeyUp('\t'); else kb_handler->OnKeyUp(0xFF); break; //Tab
        case 0x90: if (!shift) kb_handler->OnKeyUp('q'); else kb_handler->OnKeyUp('Q'); break;
        case 0x91: if (!shift) kb_handler->OnKeyUp('w'); else kb_handler->OnKeyUp('W'); break;
        case 0x92: if (!shift) kb_handler->OnKeyUp('e'); else kb_handler->OnKeyUp('E'); break;
        case 0x93: if (!shift) kb_handler->OnKeyUp('r'); else kb_handler->OnKeyUp('R'); break;
        case 0x94: if (!shift) kb_handler->OnKeyUp('t'); else kb_handler->OnKeyUp('T'); break;
        case 0x95: if (!shift) kb_handler->OnKeyUp('y'); else kb_handler->OnKeyUp('Y'); break;
        case 0x96: if (!shift) kb_handler->OnKeyUp('u'); else kb_handler->OnKeyUp('U'); break;
        case 0x99: if (!shift) kb_handler->OnKeyUp('p'); else kb_handler->OnKeyUp('P'); break;
        case 0x97: if (!shift) kb_handler->OnKeyUp('i'); else kb_handler->OnKeyUp('I'); break;
        case 0x98: if (!shift) kb_handler->OnKeyUp('o'); else kb_handler->OnKeyUp('O'); break;
        case 0x9E: if (!shift) kb_handler->OnKeyUp('a'); else kb_handler->OnKeyUp('A'); break;
        case 0x9F: if (!shift) kb_handler->OnKeyUp('s'); else kb_handler->OnKeyUp('S'); break;
        case 0xA0: if (!shift) kb_handler->OnKeyUp('d'); else kb_handler->OnKeyUp('D'); break;
        case 0xA1: if (!shift) kb_handler->OnKeyUp('f'); else kb_handler->OnKeyUp('F'); break;
        case 0xA2: if (!shift) kb_handler->OnKeyUp('g'); else kb_handler->OnKeyUp('G'); break;
        case 0xA3: if (!shift) kb_handler->OnKeyUp('h'); else kb_handler->OnKeyUp('H'); break;
        case 0xA4: if (!shift) kb_handler->OnKeyUp('j'); else kb_handler->OnKeyUp('J'); break;
        case 0xA5: if (!shift) kb_handler->OnKeyUp('k'); else kb_handler->OnKeyUp('K'); break;
        case 0xA6: if (!shift) kb_handler->OnKeyUp('l'); else kb_handler->OnKeyUp('L'); break;
        case 0xA7: if (!shift) kb_handler->OnKeyUp(';'); else kb_handler->OnKeyUp(':'); break;
        case 0xA8: if (!shift) kb_handler->OnKeyUp('\"'); else kb_handler->OnKeyUp('\''); break;
        case 0xAC: if (!shift) kb_handler->OnKeyUp('z'); else kb_handler->OnKeyUp('Z'); break;
        case 0xAD: if (!shift) kb_handler->OnKeyUp('x'); else kb_handler->OnKeyUp('X'); break;
        case 0xAE: if (!shift) kb_handler->OnKeyUp('c'); else kb_handler->OnKeyUp('C'); break;
        case 0xAF: if (!shift) kb_handler->OnKeyUp('v'); else kb_handler->OnKeyUp('V'); break;
        case 0xB0: if (!shift) kb_handler->OnKeyUp('b'); else kb_handler->OnKeyUp('B'); break;
        case 0xB1: if (!shift) kb_handler->OnKeyUp('n'); else kb_handler->OnKeyUp('N'); break;
        case 0xB2: if (!shift) kb_handler->OnKeyUp('m'); else kb_handler->OnKeyUp('M'); break;
        case 0xB3: if (!shift) kb_handler->OnKeyUp(','); else kb_handler->OnKeyUp('<'); break;
        case 0xB4: if (!shift) kb_handler->OnKeyUp('.'); else kb_handler->OnKeyUp('>'); break;
        case 0xB5: if (!shift) kb_handler->OnKeyUp('/'); else kb_handler->OnKeyUp('?'); break;
        
        case 0xB9: kb_handler->OnKeyUp(' '); break;
        case 0x8E: kb_handler->OnKeyUp('\b'); break;
        case 0x9c: kb_handler->OnKeyUp('\n'); break;

        case 0x2A: case 0x36: {shift = true; Modifiers[SHIFTKEY] = SHIFTKEY; break;}
        case 0xAA: case 0xB6: {shift = false; Modifiers[SHIFTKEY] = NOKEY; break;}

        case 0x1D: {Modifiers[CONTROLKEY] = CONTROLKEY; break;}
        case 0x9D: {Modifiers[CONTROLKEY] = NOKEY; break;}
        
        case 0x38: {Modifiers[ALTKEY] = ALTKEY; break;}
        case 0xB8: {Modifiers[ALTKEY] = NOKEY; break;}
        
        case 0x5B: {Modifiers[WINKEY] = WINKEY; break;}
        case 0xDB: {Modifiers[WINKEY] = NOKEY; break;}
        case 0xE0: break;       //I don't know why win key generates this..
        
        case 0x45: break;
        case 0x4D: kb_handler->OnKeyDown(0xF9); break;   //Right  
        case 0x4B: kb_handler->OnKeyDown(0xf8); break;   //left
        case 0x48: kb_handler->OnKeyDown(0xf7); break;   //Up
        case 0x50: kb_handler->OnKeyDown(0xf6); break;   //Down

        case 0x1: kb_handler->OnKeyDown(0xf3); break;   //Esc
        // case 0x81: esc released

        default:
            break;
    }

    return esp;
}

void ConsoleKeyboardHandler::OnKeyDown(char c)
{
    if (base->kb_handler == this)
    {
        out[0] = c;
        out[1] = 0;
    }
    print((char*)out);
}

void ConsoleKeyboardHandler::OnKeyUp(char c)
{

}