#ifndef PANASONICRAW_H
#define PANASONICRAW_H

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "Finder.h"
#include "constants.h"

inline std::string make_audio_name(const std::string & file_name, DWORD number)
{
	const int buff_size = 3;
	std::string new_name = file_name;
	char buff[buff_size];

	sprintf_s(buff, buff_size, "%.2d", number);

	new_name.append(buff);
	new_name += ".mxf";

	return new_name;
}

inline DWORD calculate_value(BYTE *buffer , const DWORD size , BYTE value)
{
	DWORD count = 0;
	for (DWORD iVal = 0; iVal < size; ++iVal)
	{
		if (buffer[iVal] == value)
			++count;
	}
	return count;
}


class Panasonic_Raw
{
private:
	std::string folder_;
	DWORD block_size_;


public:
	Panasonic_Raw(const DWORD block_size)
		: block_size_( block_size )
	{

	}

	//stringlist readAllFiles(const std::string & folder)
	//{
	//	FileFinder finder;
	//	stringlist freelist;
	//	finder.FindFiles(folder, freelist);
	//	return finder.getFileNames();
	//}

	bool read_cluster(HANDLE & hHandle ,BYTE * buffer , DWORD size , DWORD number)
	{
		DWORD bytes_read = 0;
		DWORD offset = number * block_size_;
		IO::set_position(hHandle, offset);
		if (!IO::read_block(hHandle, buffer, size, bytes_read))
			return false;

		if (bytes_read == size)
			return true;
			
		return false;
	}
	bool write_cluster(HANDLE & hHandle, BYTE * buffer, DWORD size)
	{
		DWORD bytesWritten = 0;
		if (!IO::write_block(hHandle, buffer, size, bytesWritten))
			return false;

		if (bytesWritten == size)
			return true;
		return false;
	}

	void analyze_file(const std::string & file_path, const std::string target_folder)
	{
		HANDLE hSource = INVALID_HANDLE_VALUE;
		if (!IO::open_read(hSource, file_path))
		{
			printf("Error to open file %s\r\n", file_path.c_str());
			return;
		}

		LONGLONG file_size = IO::getFileSize(hSource);
		if (file_size == 0)
		{
			printf("Error file size is 0\r\n");
			return;
		}

		LONGLONG cluster_count = file_size / block_size_;

		HANDLE hVideo = INVALID_HANDLE_VALUE;
		HANDLE hAudio00 = INVALID_HANDLE_VALUE;
		HANDLE hAudio01 = INVALID_HANDLE_VALUE;

		fs::path path(file_path);
		std::string source_name = path.filename().generic_string();

		std::string video_file = IO::make_file_path(target_folder, source_name + ".mxf");
		if (!IO::create_file(hVideo, video_file))
		{
			printf("Error to create file %s.\r\n", video_file.c_str());
			return;
		}

		std::string audio0_file = IO::make_file_path(target_folder, make_audio_name(source_name, 0));
		if (!IO::create_file(hAudio00, audio0_file))
		{
			printf("Error to create file %s.\r\n", audio0_file.c_str());
			return;
		}
		std::string audio1_file = IO::make_file_path(target_folder, make_audio_name(source_name, 1));
		if (!IO::create_file(hAudio01, audio1_file))
		{
			printf("Error to create file %s.\r\n", audio1_file.c_str());
			return;
		}

		bool bResult = false;
		BYTE * buffer = new BYTE[block_size_];

		BYTE * header_buffer = new BYTE[block_size_];
		BYTE * header_audio00 = new BYTE[block_size_];
		BYTE * header_audio01 = new BYTE[block_size_];

		DWORD header_cluster = cluster_count - 2;
		if (read_cluster(hSource, buffer, block_size_, header_cluster))
		{
			memcmp(header_buffer, buffer, block_size_);
			if (!write_cluster(hVideo, buffer, block_size_))
			{
				printf("Error write cluster");
				return;
			}
		}


		DWORD header_counter = 1;
		DWORD audio_counter = 0;
		DWORD audio_cluster = 6;

		for (DWORD iCluster = 3; iCluster < cluster_count; ++iCluster)
		{
			if (!read_cluster(hSource, buffer , block_size_ , iCluster))
			{
				printf("Error read cluster");
				return;
			}
			if (isHeaderCluster(buffer, block_size_))
			{
				switch (header_counter)
				{
				case 1:
					write_cluster(hAudio00, buffer, block_size_);
					break;
				case 2:
					write_cluster(hAudio01, buffer, block_size_);
					break;
				case 3:
					memcpy(header_audio00, buffer, block_size_);
					break;
				case 4:
					memcpy(header_audio01, buffer , block_size_);
					break;
				}

				++header_counter;
			}
			else
			{
				if (calculate_value(buffer, block_size_, 0x9F) > 300)
					write_cluster(hVideo, buffer, block_size_);
				else
				{
					if (audio_counter == 8)
						audio_counter = 0;


					if (audio_cluster > 0)
					{
						if (audio_counter < 3)
							write_cluster(hAudio00, buffer, block_size_);
						else
							write_cluster(hAudio01, buffer, block_size_);

						--audio_cluster;
						if (audio_cluster == 0)
						{
							audio_counter = 0;
							continue;
						}


					}
					else
					{
						if (audio_counter < 4)
							write_cluster(hAudio00, buffer, block_size_);
						else
							write_cluster(hAudio01, buffer, block_size_);
					}

					++audio_counter;
				}
			}
		}

		IO::set_position(hAudio00, 0);
		write_cluster(hAudio00, header_audio00,block_size_);

		IO::set_position(hAudio01, 0);
		write_cluster(hAudio01, header_audio01, block_size_);


		delete[] header_buffer;
		delete[] header_audio00;
		delete[] header_audio01;
		delete[] buffer;
		CloseHandle(hVideo);
		CloseHandle(hAudio00);
		CloseHandle(hAudio01);
		CloseHandle(hSource);

		
	}
	void execute(const std::string source_folder , const std::string & target_folder)
	{
		//stringlist files_path = readAllFiles(source_folder);

		//auto iter = files_path.begin();
		//while (iter != files_path.end())
		//{
		//	analyze_file(*iter, target_folder);
		//	++iter;
		//}

	}

	bool isHeaderCluster(BYTE * buffer, DWORD size)
	{
		if (Signatures::mxf_header_size > size)
			return false;

		return (memcmp(buffer, Signatures::mxf_header, Signatures::mxf_header_size) == 0);
	}

};


#endif