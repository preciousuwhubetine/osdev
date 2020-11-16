#ifndef CRYSTALOS__IO__PORT_H
#define CRYSTALOS__IO__PORT_H

#include <common/types.h>

namespace crystalos
{
	namespace IO
	{
		class Port
		{
			protected:
				common::uint16_t portnumber;
				Port(common::uint16_t portnumber);
				~Port();
		};
		
		class Port8Bit : public Port
		{
			public:
				Port8Bit(common::uint16_t portnumber);
				~Port8Bit();
				virtual void Write(common::uint8_t data);
				virtual common::uint8_t Read();
		};
		
		class Port8BitSlow : public Port8Bit
		{
			public:
				Port8BitSlow(common::uint16_t portnumber);
				~Port8BitSlow();
				virtual void Write(common::uint8_t data);
		};
		
		class Port16Bit : public Port
		{
			public:
				Port16Bit(common::uint16_t portnumber);
				~Port16Bit();
				virtual void Write(common::uint16_t data);
				virtual common::uint16_t Read();
		};

		class Port32Bit : public Port
		{
			public:
				Port32Bit(common::uint16_t portnumber);
				~Port32Bit();
				virtual void Write(common::uint32_t data);
				virtual common::uint32_t Read();
		};
	}
}
#endif