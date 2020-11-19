#ifndef CRYSTALOS__DRIVERS__DRIVER_H
#define CRYSTALOS__DRIVERS__DRIVER_H

/* 
	All kernel drivers extend this class
*/

namespace crystalos
{
	namespace drivers
	{
		class Driver
		{
			public:
				Driver();
				~Driver();
				
				virtual void Activate();
				virtual int Reset();
				virtual void Deactivate();
		};

		class DriverManager
		{
			public:
				Driver* drivers[256];
				int numDrivers;
				
			public:
				DriverManager();
				void Add(Driver* drv);	
				void ActivateAll();
		};
	}
}

#endif