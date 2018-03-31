#pragma once

#include "constants.h"
#include "IODevice.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <locale>
#include <codecvt>
//#include <boost\variant\variant.hpp>

namespace IO
{
	inline path_string toWString(const std::string & oneByteString)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(oneByteString);

	}
	inline path_string addBackSlash(const path_string & path_str)
	{
		path_string new_string(path_str);
		if (*path_str.rbegin() != back_slash)
			new_string.push_back(back_slash);

		return new_string;
	}

	inline bool createFoldersFromPath(const path_string & path)
	{

		return false;
	}
	inline bool isDirectoryAttribute(const WIN32_FIND_DATA & attributes)
	{
		return (attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	inline bool isOneDotOrTwoDots(const path_string & name_string)
	{
		if (name_string.compare(OneDot) == 0)
			return true;
		if (name_string.compare(TwoDot) == 0)
			return true;
		return false;
	}


	inline bool isPresentInList(const path_string & text_value, const path_list & listToFind)
	{
		auto str = text_value;
		boost::algorithm::to_lower(str);
		//auto findIter = std::find(listToFind.begin(), listToFind.end(), str);
		for (auto & theExt : listToFind)
		{
			if (theExt.compare(str) == 0)
				return true;
		}
		//return (findIter != listToFind.end()) ? true : false;
		return false;
	}

	inline path_string getExtension(const path_string & file_name)
	{
		boost::filesystem::path file_path(file_name);	// can crash ????
		return file_path.extension().generic_wstring();
	}

	inline bool isOffice2003(const path_string & file_extension)
	{
		return isPresentInList(file_extension, listExtensions2003);
	}

	inline bool isOffice2007(const path_string & file_extension)
	{
		return isPresentInList(file_extension, listExtensions2007);
	}

	inline void JoinWithService(const path_string & data_file, 
								const path_string & service_file,
								const path_string & target_file,
								const uint32_t data_size,
								const uint32_t service_size)
	{
		const uint32_t target_page = data_size + service_size;
		
		File f_data(data_file);
		if (!f_data.Open(OpenMode::OpenRead))
		{
			wprintf_s(L"Error open file. %s\n", f_data.getFileName().c_str());
			return;
		}

		File f_sevice(service_file);
		if (!f_sevice.Open(OpenMode::OpenRead))
		{
			wprintf_s(L"Error open file. %s\n", f_sevice.getFileName().c_str());
			return;
		}

		File f_target(target_file);
		if (!f_target.Open(OpenMode::Create))
		{
			wprintf_s(L"Error open file. %s\n", f_target.getFileName().c_str());
			return;
		}

		
		Buffer data_buffer(data_size);
		Buffer service_buffer(service_size);
		Buffer target_buffer(target_page);

		bool bResult = false;

		LONGLONG max_size = f_data.Size();

		LONGLONG cur = 0;
		while (cur < max_size)
		{
			if ( f_data.ReadData(data_buffer.data, data_buffer.data_size) == 0 )
			{
				wprintf_s(L"Error read data file.\n");
				break;
			}

			
			if (f_sevice.ReadData(service_buffer.data, service_buffer.data_size) == 0)
			{
				wprintf_s(L"Error read service file.\n");
				break;
			}

			memcpy(target_buffer.data, data_buffer.data, data_buffer.data_size);
			memcpy(target_buffer.data + data_buffer.data_size, service_buffer.data, service_buffer.data_size);

			if (f_target.WriteData(target_buffer.data, target_buffer.data_size) == 0)
			{
				wprintf_s(L"Error write  file.\n");
				break;
			}


			cur += data_size;

		}


	}

	inline path_string toNumberString(const uint32_t number)
	{
		const int numValues = 11;
		wchar_t buff[numValues] = {0};
		ZeroMemory(buff, sizeof(wchar_t) * numValues);

		swprintf_s(buff, numValues, L"%.10u", number);
		path_string number_str(buff);
		return number_str;
	}

	inline path_string toHexString(const uint64_t big_value)
	{
		const int numValues = 17;
		wchar_t buff[numValues] = { 0 };
		ZeroMemory(buff, sizeof(wchar_t) * numValues);

		swprintf_s(buff, numValues, L"%.15I64x", big_value);
		path_string number_str(buff);
		return number_str;
	}

	inline path_string toNumberExtension(const uint32_t number, const path_string & extension)
	{
		return toNumberString(number) + extension;
	}
	inline path_string toFullPath(const path_string & folder, const uint32_t number, const path_string & extension)
	{
		return addBackSlash(folder) + toNumberString(number) + extension;
	}
	inline path_string offsetToPath(const path_string & folder, const uint64_t byte_offset, const path_string & extension, uint32_t sector_size = 512)
	{
		return addBackSlash(folder) + toHexString(byte_offset / sector_size) + extension;
	}

	inline bool createDirectory(const path_string & folder, const path_string & new_folder, path_string & result_folder)
	{
		result_folder = addBackSlash(folder) + new_folder;
		if (!boost::filesystem::exists(result_folder))
			return boost::filesystem::create_directory(result_folder);
	}

	inline std::string getDateFromSystemtime(const SYSTEMTIME & system_time)
	{
		char tmp_buffer[255];
		::GetDateFormatA(LOCALE_USER_DEFAULT, 0, &system_time, "yyyy-MM-dd", tmp_buffer, 255);
		std::string tmp_str = tmp_buffer;

		return tmp_str;
	}

	inline std::string getTimeFromSystemtime(const SYSTEMTIME & system_time)
	{
		char tmp_buffer[255];
		::GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &system_time, "HH-mm-ss", tmp_buffer, 255);
		std::string tmp_str = tmp_buffer;

		return tmp_str;
	}

	inline std::string getDateAndTimeFromSystemtime(const SYSTEMTIME & system_time)
	{
		return getDateFromSystemtime(system_time) + '-' + getTimeFromSystemtime(system_time);
	}

	inline std::string parse_string_date(const std::string & original_date)
	{
		// 2017-06-02T13:25:56+03:00
		SYSTEMTIME sys_time = { 0 };
		std::string tmp_str;
		sys_time.wYear = std::stoi(original_date.substr(0, 4));
		sys_time.wMonth = std::stoi(original_date.substr(5, 2));
		sys_time.wDay = std::stoi(original_date.substr(8, 2));
		sys_time.wHour = std::stoi(original_date.substr(11, 2));
		sys_time.wMinute = std::stoi(original_date.substr(14, 2));
		sys_time.wSecond = std::stoi(original_date.substr(17, 2));

		return getDateAndTimeFromSystemtime(sys_time);
	}

	//inline std::string getQuickTimeDate(const IO::path_string & filePath)
	//{

	//}
	struct CanonDate
	{
		//2017:08:08 11:10:02
		char year[4];
		char colon1;
		char month[2];
		char colon2;
		char day[2];
		char white_space;
		char hour[2];
		char colon3;
		char min[2];
		char colon4;
		char sec[2];

	};

	inline uint32_t calc_nulls(const ByteArray data, const uint32_t size)
	{
		uint32_t counter = 0;
		for (uint32_t iByte = 0; iByte < size; ++iByte)
		{
			if (data[iByte] == 0)
				++counter;
		}
		return counter;
	}

	inline void RenameMP4_date(const IO::path_string & filePath)
	{
		//auto test_file = IO::makeFilePtr(filePath);
		//const uint32_t date_offset = 0x126;
		//const uint32_t date_size = 19;
		//const uint32_t check_size = 0x13A;
		//char buff[date_size + 1];
		//ZeroMemory(buff, date_size + 1);

		//if (test_file->Open(IO::OpenMode::OpenRead))
		//{
		//	if (check_size > test_file->Size())
		//		return;

		//	test_file->setPosition(date_offset);
		//	test_file->ReadData((ByteArray)buff, date_size);


		//	if (buff[0] == '2' && buff[1] == '0')
		//	{
		//		// Canon date
		//		std::string date_string(buff);
		//		std::replace(date_string.begin(), date_string.end(), ' ', '-');
		//		std::replace(date_string.begin(), date_string.end(), '.', '-');
		//		std::replace(date_string.begin(), date_string.end(), ':', '-');

		//		IO::path_string new_date_str(date_string.begin(), date_string.end());

		//		boost::filesystem::path src_path(filePath);
		//		auto folder_path = src_path.parent_path().generic_wstring();
		//		auto only_name_path = src_path.stem().generic_wstring();
		//		auto ext = src_path.extension().generic_wstring();
		//		auto new_name = folder_path + L"//" + new_date_str + L"-" + only_name_path + ext;
		//		test_file->Close();
		//		try
		//		{
		//			boost::filesystem::rename(filePath, new_name);
		//		}
		//		catch (const boost::filesystem::filesystem_error& e)
		//		{
		//			std::cout << "Error: " << e.what() << std::endl;
		//		}

		//	}
		//	else
		//	{
		//		// Sony xml creation date
		//		const std::string temp_xml = "temp.xml";
		//		std::wstring wtemp_xml = L"temp.xml";
		//		auto tmp_file = makeFilePtr(wtemp_xml);
		//		tmp_file->Open(OpenMode::Create);

		//		const char xml_header[] = { 0x3C , 0x3F ,  0x78 , 0x6D , 0x6C };
		//		const uint32_t xml_header_size = SIZEOF_ARRAY(xml_header);

		//		char buffer[default_block_size];
		//		ZeroMemory(buffer, default_block_size);

		//		auto file_size = test_file->Size();
		//		test_file->setPosition(file_size - default_block_size);
		//		auto bytes_read = test_file->ReadData((ByteArray)buffer, default_block_size);
		//		if (bytes_read == 0)
		//		{
		//			wprintf(L"Error. Read size is 0");
		//			return;
		//		}

		//		bool bFoudXml = false;
		//		// Find xml signauture
		//		for (uint32_t iByte = 0; iByte < bytes_read - xml_header_size; ++iByte)
		//		{
		//			if (memcmp(buffer + iByte, xml_header, xml_header_size) == 0)
		//			{
		//				// write to temp file
		//				tmp_file->WriteData((ByteArray)&buffer[iByte], bytes_read - iByte);
		//				tmp_file->Close();
		//				bFoudXml = true;
		//			}
		//		}

		//		if (bFoudXml)
		//		{
		//			tinyxml2::XMLDocument xml_doc;
		//			auto xml_result = xml_doc.LoadFile(temp_xml.c_str());

		//			if (xml_result == tinyxml2::XML_SUCCESS)
		//			{
		//				auto xml_root = xml_doc.RootElement();
		//				auto xmlCreationDate = xml_root->FirstChildElement("CreationDate");
		//				if (xmlCreationDate)
		//				{
		//					auto xml_date_text = xmlCreationDate->Attribute("value");
		//					if (xml_date_text)
		//					{
		//						std::string original_date(xml_date_text);
		//						auto str_date = parse_string_date(xml_date_text);
		//						std::cout << str_date.c_str();

		//						IO::path_string new_date_str(str_date.begin(), str_date.end());

		//						boost::filesystem::path src_path(filePath);
		//						auto folder_path = src_path.parent_path().generic_wstring();
		//						auto only_name_path = src_path.stem().generic_wstring();
		//						auto ext = src_path.extension().generic_wstring();
		//						auto new_name = folder_path + L"//" + new_date_str + L"-" + only_name_path + ext;

		//						test_file->Close();
		//						try
		//						{
		//							boost::filesystem::rename(filePath, new_name);
		//						}
		//						catch (const boost::filesystem::filesystem_error& e)
		//						{
		//							std::cout << "Error: " << e.what() << std::endl;
		//						}

		//					}
		//				}
		//				//auto val_text = xml_date_element->ToText();

		//			}
		//		}

		//	}

		//}

	}

	inline bool isDigitString(const std::string digit_string)
	{
		for (size_t i = 0; i < digit_string.length(); ++i)
			if (isdigit(digit_string.at(i)) == 0)
				return false;
		return true;
	}
	// path ....img016937500.bin
	inline uint64_t fileNameToOffset(const IO::path_string & filePath, uint32_t skip_size = 3)
	{
		boost::filesystem::path src_path(filePath);
		//auto folder_path = src_path.parent_path().generic_string();
		auto only_name_path = src_path.stem().generic_string();
		//auto ext = src_path.extension().generic_string();
		
		uint64_t file_offset = 0;
		std::string digits_string(only_name_path.substr(skip_size));
		if (isDigitString(digits_string))
		{
			file_offset = std::stoll(digits_string);
			
		}
		else
		{
			printf("Error to parse file \n");
		}


		return file_offset;
	}



	const uint8_t EIGHT_FF[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF , 0x78 , 0xDA };
	const uint32_t EIGHT_FF_SIZE = SIZEOF_ARRAY(EIGHT_FF);

	const uint32_t ACRONIS_BLOCK_SIZE = 256 * 1024;

	class AcronisDecompress
	{
	private:
		File file_;
	public:
		AcronisDecompress(const path_string & acronis_file)
			: file_(acronis_file)
		{}
		uint64_t find_8FF(const uint64_t start_offset )
		{
			uint64_t offset = start_offset;
			DataArray data_array(ACRONIS_BLOCK_SIZE);
			uint32_t bytes_read = 0;

			while (offset < file_.Size())
			{
				file_.setPosition(offset);
				bytes_read = file_.ReadData(data_array.data(), data_array.size());
				if (bytes_read == 0)
					break;

				for (uint32_t iByte = 0; iByte < bytes_read - EIGHT_FF_SIZE; ++iByte)
					if (memcmp(data_array.data() + iByte, EIGHT_FF, EIGHT_FF_SIZE) == 0)
					{
						return offset + iByte;
					}

				offset += bytes_read;
			}
			return start_offset;
		}

		int decode_block(const uint64_t start_offset, const uint64_t end_offset, DataArray & dst_data_array);

		uint64_t saveToFile(const path_string & target_file_name, const uint64_t start_offset = 0)
		{
			if (!this->file_.Open(OpenMode::OpenRead))
			{
				wprintf(L"Error open source file.\n");
				return 0 ;
			}

			File target_file(target_file_name);
			if (!target_file.Open(OpenMode::Create))
			{
				wprintf(L"Error Create target file.\n");
				return 0 ;
			}

			uint64_t target_offset = 0;
			uint64_t ff_start = find_8FF(start_offset);
			uint64_t offset = ff_start + 8;
			while (true)
			{
				wprintf(L"offset: %I64d\n", offset);
				auto new_block_offset = find_8FF(offset);
				if (new_block_offset == offset)
				{
					wprintf(L"Not found next 0xFFFFFFFFFF\n");
					return 0 ;
				}

				DataArray decompressed_data(ACRONIS_BLOCK_SIZE);
				auto iRes = decode_block(offset, new_block_offset, decompressed_data);
				if (iRes != 0)
				{
					wprintf(L"Can't decompress data.\n");
					return new_block_offset;
				}
				target_file.setPosition(target_offset);
				if ( target_file.WriteData(decompressed_data.data(), decompressed_data.size()) == 0)
				{
					wprintf(L"Error write to file.\n");
				}
				target_offset += decompressed_data.size();

				offset = new_block_offset + EIGHT_FF_SIZE - 2;
			}
			return offset;
		}

	};


	inline uint64_t alingToValue(const uint64_t theValue, const uint32_t toValue)
	{
		if (toValue == 0)
			return 0;
		uint64_t resValue = theValue / toValue;
		resValue *= toValue;
		return resValue;
	}

	inline uint64_t alingToSector(const uint64_t offset, const uint32_t sector_size = default_sector_size)
	{
		return alingToValue(offset, sector_size);
	}
};