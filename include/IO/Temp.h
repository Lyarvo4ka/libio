#pragma once

//inline int NotNullPosFromEnd(const Buffer & buffer)
//{
//	int pos = buffer.data_size - 1;
//	while (pos >= 0)
//	{
//		if (buffer.data[pos] != 0)
//			return (buffer.data_size - pos);
//		--pos;
//	}
//	return 0;
//}

//inline void removeNullsFromEndFile(const path_string & file_path, uint32_t sizeToTest = 4096)
//{
//	File file(file_path);
//	if (!file.Open(OpenMode::OpenWrite))
//	{
//		wprintf_s(L"Error open file.\n");
//		return;
//	}
//
//	auto file_size = file.Size();
//	Buffer buffer(sizeToTest);
//	if (file_size >= sizeToTest)
//	{
//		uint64_t lastBlock = file_size /*- sizeToTest*/;
//		file.setPosition(0);
//		auto bytesRead = file.ReadData(buffer.data, buffer.data_size);
//		if (bytesRead == sizeToTest)
//		{
//			int not_null = NotNullPosFromEnd(buffer);
//			if (not_null > 0)
//			{
//				uint64_t new_size = file_size - not_null + 1;
//				file.setSize(new_size);
//				wprintf_s(L"FIle size has been changed %s.\n", file_path.c_str());
//			}
//		}
//	}
//
//}

//inline void zbk_rename(const path_string & file_path)
//{
//	Signatures::zbh_header_t zbk_header = Signatures::zbh_header_t();
//
//	File file(file_path);
//	if (!file.Open(OpenMode::OpenWrite))
//	{
//		wprintf_s(L"Error open file.\n");
//		return;
//	}
//
//	auto bytes_read = file.ReadData((IO::ByteArray)&zbk_header, sizeof(Signatures::zbh_header_t));
//	//		zbk_header.year = zbk_header.year >> 8 | zbk_header.year << 8;
//
//	wchar_t buff[1024];
//	ZeroMemory(buff, 1024 * sizeof(wchar_t));
//
//	swprintf_s(buff, 1024, L"%d-%.2d-%.2d-%.2d-%.2d", zbk_header.year, zbk_header.month, zbk_header.day, zbk_header.hour, zbk_header.seconds);
//	path_string date_str = buff;
//
//	boost::filesystem::path src_path(file_path);
//	auto folder_path = src_path.parent_path().generic_wstring();
//	auto only_name_path = src_path.stem().generic_wstring();
//	auto ext = src_path.extension().generic_wstring();
//	auto new_name = folder_path + L"//" + date_str + L"---" + only_name_path + ext;
//
//	file.Close();
//	try
//	{
//		boost::filesystem::rename(file_path, new_name);
//	}
//	catch (const boost::filesystem::filesystem_error& e)
//	{
//		std::cout << "Error: " << e.what() << std::endl;
//	}
//
//
//}

//inline void addDateToFile(const path_string & file_path)
//{
	//File file(file_path);
	//if (!file.Open(OpenMode::OpenWrite))
	//{
	//	wprintf_s(L"Error open file.\n");
	//	return;
	//}

	//const char cmp_to[] = { 0xC2 , 0xF0 , 0xE5 , 0xEC , 0xFF , 0x3A };

	//auto file_size = file.Size();
	//Buffer buffer(file_size);
	//auto bytesRead = file.ReadData(buffer.data, buffer.data_size);
	//file.Close();

	//bool bFound = false;
	//uint32_t offset_pos = 0;
	//for (offset_pos = 0; offset_pos < file_size; ++offset_pos)
	//	if (memcmp(buffer.data + offset_pos, cmp_to, SIZEOF_ARRAY(cmp_to)) == 0 )
	//	{
	//		bFound = true;
	//		break;
	//	}


	//if (!bFound)
	//{
	//	wprintf_s(L"Not found string!!!!.\n");
	//	return;
	//}
	//const uint32_t date_size = 19 + 1;
	////const uint32_t date_offset = 0x37;
	//char raw_date[date_size];
	//ZeroMemory(raw_date, date_size);

	//memcpy(raw_date, buffer.data + offset_pos + SIZEOF_ARRAY(cmp_to) + 1, date_size - 1);

	//std::string date_string(raw_date);
	//std::replace(date_string.begin(), date_string.end(), ' ', '-');
	//std::replace(date_string.begin(), date_string.end(), '.', '-');
	//std::replace(date_string.begin(), date_string.end(), ':', '-');

	//IO::path_string new_date_str(date_string.begin(), date_string.end());

	//boost::filesystem::path src_path(file_path);
	//auto folder_path = src_path.parent_path().generic_wstring();
	//auto only_name_path = src_path.stem().generic_wstring();
	//auto ext = src_path.extension().generic_wstring();
	//auto new_name = folder_path + L"//" + only_name_path + L"-" + new_date_str + ext;

	//try
	//{
	//	boost::filesystem::rename(file_path, new_name);
	//}
	//catch (const boost::filesystem::filesystem_error& e)
	//{
	//	std::cout << "Error: " << e.what() << std::endl;
	//}

	//
	//int k = 1;
	//k = 1;
//}



//struct pck_t
//{
//	char header[3];
//	char reserved[29];
//	char digits[15];
//	char text[33];
//};
//
//std::string getNameFromPCKFileAndChangeSize(const path_string & file_path)
//{
//	static int counter = 0;
//	++counter;
//	File file(file_path);
//	if (!file.Open(OpenMode::OpenWrite))
//	{
//		wprintf_s(L"Error open file.\n");
//		return std::to_string(counter);
//	}
//	std::string new_name = std::to_string(counter);
//
//	auto file_size = file.Size();
//	if (file_size > 0)
//	{
//		uint32_t bytes_read = 0;
//		const int pck_size = sizeof(pck_t);
//
//		Buffer buffer((uint32_t)file_size);
//		bytes_read = file.ReadData(buffer.data, buffer.data_size);
//		if (bytes_read != file_size)
//			return std::to_string(counter);
//
//		pck_t * pck_header = (pck_t *)buffer.data;
//
//		std::string digit_name(pck_header->digits);
//		std::string text_name(pck_header->text);
//
//		new_name = digit_name + "_"/* + text_name + "_"*/ + std::to_string(counter);
//		boost::algorithm::erase_all(new_name, "_"); // '\"'
//													//new_name.erase(std::remove_if(new_name.begin(), new_name.end(), '\\'), new_name.end());
//													//std::remove_if(new_name.begin(), new_name.end(), );
//
//		int pos = (int)(file_size - 1);
//		while (pos != 0)
//		{
//			if (buffer.data[pos] != 0)
//				break;
//			--pos;
//		}
//
//		file.setSize(pos + 1);
//	}
//
//	return new_name;
//}
//
