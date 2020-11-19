#include <util/system.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::IO;
using namespace crystalos::util;

//Sleeping or scheduling for 20 is 1 second.
//10 - 500ms
//5 - 250ms
//2 - 120ms

void util::Sleep(int ms)
{
	InterruptManager::ActiveInterruptManager->sleepCounter = 0;
	while (InterruptManager::ActiveInterruptManager->sleepCounter < ms)
	{
		//Just wait
	}
}

bool util::Schedule(void (*handler)(), int numFrames)
{
	InterruptManager::ActiveInterruptManager->Schedule(handler, numFrames);
}