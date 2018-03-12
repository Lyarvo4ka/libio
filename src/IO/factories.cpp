#include "IO\factories.h"
#include "IO\AbstractRaw.h"
#include "IO\StandartRaw.h"
#include "IO\QuickTime.h"

namespace IO
{
	void RawFactoryManager::Register(const std::string & algorithmName, RawFactoryPtr rawFactory)
	{
		if (factories_.find(algorithmName) == factories_.end())
		{
			factories_.emplace(algorithmName, std::move(rawFactory));
		}
	}

	IO::RawFactory * RawFactoryManager::Lookup(const std::string & algorithmName)
	{
		auto findIter = factories_.find(algorithmName);
		if (findIter != factories_.end())
		{
			return findIter->second.get();
		}
		return nullptr;
	}

};
