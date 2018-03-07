#pragma once

#include "iolibrary_global.h"
#include "IODevice.h"
#include <map>
#pragma warning(disable:4251)

namespace IO
{
	class RawAlgorithm;

	class RawFactory
	{
	public:
		virtual ~RawFactory() {}
		virtual RawAlgorithm * createRawAlgorithm(IODevicePtr) = 0;
	};

	using RawFactoryPtr = std::unique_ptr<RawFactory>;

	class RawFactoryManager
	{
		using FactoryMap = std::map<std::string, RawFactoryPtr>;
	private:
		FactoryMap factories_;
	public:

		void Register(const std::string & algorithmName, RawFactoryPtr  rawFactory);
		RawFactory * Lookup(const std::string & algorithmName);
	};
};