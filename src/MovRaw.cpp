#include "MovRaw.h"
//
//CanonMovRaw::CanonMovRaw(const std::string & file_name, const std::string & output_folder) 
//: MovRaw(file_name, output_folder)
//, cluster_size_(0)
//, offset_(0)
//{
//
//}
//
//CanonMovRaw::CanonMovRaw(const DWORD number, const std::string & output_folder) : MovRaw(number, output_folder)
//, cluster_size_(0)
//, offset_(0)
//{
//
//}
//
//void CanonMovRaw::show_error(const std::string & error_text)
//{
//	printf(error_text.c_str());
//}
//
//bool CanonMovRaw::cmp_mdat(const BYTE * data)
//{
//	return (memcmp(&data[4], Signatures::mdat, Signatures::mdat_size) == 0);
//}
//
//bool CanonMovRaw::find_mdat(const BYTE * data, const int data_size, int & offset)
//{
//	for (int iSector = 0; iSector < data_size; iSector += SECTOR_SIZE)
//	{
//		if (cmp_mdat(&data[iSector]))
//		{
//			offset = iSector;
//			return true;
//		}
//
//	}
//	return false;
//}
//
//void CanonMovRaw::setClusterSize(const DWORD cluster_size)
//{
//	cluster_size_ = cluster_size;
//}
//
//void CanonMovRaw::setOffset(const LONGLONG offset)
//{
//	offset_ = offset;
//}
//
//void CanonMovRaw::execute()
//{
//	if (!check_parameters())
//		return;
//
//	LONGLONG pos = offset_;
//	BYTE *data_buffer = new BYTE[cluster_size_];
//	DWORD bytesRead = 0;
//
//	auto hSource = this->getHandle();
//
//	LONGLONG mdat_offset = 0;
//	int block_offset = 0;
//
//	int counter = 0;
//
//	while (true)
//	{
//		IO::set_position(*hSource, pos);
//		if (!IO::read_block(*hSource, data_buffer, cluster_size_, bytesRead))
//			break;
//
//		if (bytesRead == 0)
//			break;
//
//		
//		if (find_mdat(data_buffer, bytesRead, block_offset))
//		{
//			InfoPartData mdat_data;
//			mdat_data.offset = (LONGLONG)pos + (LONGLONG)block_offset;
//			mdat_data.size = getMdatSize(&data_buffer[block_offset]);
//			mdat_data.size += 16;
//
//			printf("Found mdat %lld (sectors) \r\n", IO::toSectors(mdat_data.offset));
//
//			LONGLONG Header_start = IO::toSectors(mdat_data.getLastPos());
//			Header_start *= SECTOR_SIZE;
//
//			DWORD bytes_header = 0;
//			bool bFound = false;
//
//			while (true)
//			{
//				IO::set_position(*hSource, Header_start);
//				if (!IO::read_block(*hSource, data_buffer, cluster_size_, bytes_header))
//					break;
//
//				if (bytesRead == 0)
//					break;
//
//				if (find_header(data_buffer, bytes_header, block_offset))
//				{
//					InfoPartData header_data;
//					header_data.offset = (LONGLONG)Header_start + (LONGLONG)block_offset;
//					header_data.size = getDataSize(&data_buffer[block_offset]);
//					header_data.size += 24;
//
//					printf("Found header %lld (sectors) \r\n", IO::toSectors(header_data.offset));
//
//					std::string write_name(IO::file_path_number(this->folder(), counter++, ".mov"));
//
//					HANDLE hWrite = INVALID_HANDLE_VALUE;
//					if (IO::create_file(hWrite, write_name))
//					{
//						if (IO::write_block_to_file(*hSource, header_data.offset, header_data.size, hWrite))
//							if (IO::write_block_to_file(*hSource, mdat_data.offset, mdat_data.size, hWrite, header_data.size))
//							{
//								printf("Write to file: %s - OK.\r\n", write_name.c_str());
//							}
//					}
//
//					CloseHandle(hWrite);
//
//					pos = IO::toSectors(header_data.offset);
//					pos *= SECTOR_SIZE;
//					pos += SECTOR_SIZE;
//					bFound = true;
//					break;
//				}
//				else
//				{
//					pos = Header_start;
//					Header_start += cluster_size_;
//					if (find_mdat(data_buffer, bytes_header, block_offset))
//					{
//						printf("Not Found header \r\n");
//						break;
//					}
//				}
//
//			}
//			if (bFound)
//				bFound = false;
//			else
//				pos += SECTOR_SIZE;
//		}
//		else
//			pos += bytesRead;
//
//	}
//	delete[] data_buffer;
//}
//
//bool CanonMovRaw::check_parameters()
//{
//	if (!this->isReady())
//	{
//		show_error("Error. File isn't ready. \r\nProgram will be closed.\r\n");
//		return false;
//	}
//	if (cluster_size_ == 0)
//	{
//		show_error("Error. You entered wrong cluster size\r\n");
//		return false;
//	}
//	return true;
//}
