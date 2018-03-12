#ifndef R3D_RAW_H
#define R3D_RAW_H

#include "StandartRaw.h"

namespace IO
{
	class RawR3D
		: public DefaultRaw
	{
	public:
		RawR3D(IODevicePtr device)
			: DefaultRaw(device)
		{

		}



	};
}



#endif