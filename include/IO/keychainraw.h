#pragma once

#include "AbstractRaw.h"
#include "Factories.h"

namespace IO
{
#pragma pack( 1 )
	struct Keychain_struct
	{
		uint32_t sign = 0;
		uint32_t test_01 = 0;
		uint8_t skip[12];
		uint32_t size = 0;
	};
#pragma pack()
	const uint32_t Keychain_struct_size = sizeof(Keychain_struct);
	const uint32_t kych_sign = 0x6B796368;
	const uint32_t val_0100 = 0x00010000;


	class KeychainRaw
		: public DefaultRaw
	{
	public:
		KeychainRaw(IODevicePtr device)
			:DefaultRaw(device)
		{}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset)   override
		{
			Keychain_struct keychain = Keychain_struct();
			setPosition(start_offset);
			auto bytes_read = this->ReadData((ByteArray)&keychain, Keychain_struct_size);
			if (bytes_read != Keychain_struct_size)
				return 0;

			toBE32(keychain.sign);
			toBE32(keychain.size);
			toBE32(keychain.test_01);

			if (keychain.sign == kych_sign)
				if (keychain.test_01 == val_0100)
			{
				auto write_size = keychain.size + Keychain_struct_size;
				return appendToFile(target_file, start_offset, write_size);
			}
			return 0;
		}
		bool Specify(const uint64_t header_offset) override
		{
			Keychain_struct keychain = Keychain_struct();
			setPosition(header_offset);
			auto bytes_read = this->ReadData((ByteArray)&keychain, Keychain_struct_size);
			if (bytes_read != Keychain_struct_size)
				return false;

			toBE32(keychain.sign);
			toBE32(keychain.size);
			toBE32(keychain.test_01);

			if (keychain.sign == kych_sign)
				if (keychain.test_01 == val_0100)
					return true;
			return false;
		}

	};


	class KeychainRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new KeychainRaw(device);
		}
	};
}
