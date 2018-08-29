#include "IO\XorAnalyzer.h"
#include "IO\iofunctions.h"

#include "IO\constants.h"

#include <iostream>

namespace IO
{
	const int BYTE_SIZE = 256;

	ULONGLONG NumBytesForBlock(DWORD block_size)
	{
		return (LONGLONG)(sizeof(WORD) * BYTE_SIZE) * block_size;
	}

	int chunksPrerBlock(ULONGLONG block_size)
	{
		return (int)(block_size / BS::GB);
	}


	ByteCount::ByteCount(void)
	{
		bytes_ = new WORD[BYTE_SIZE];
	}

	ByteCount::~ByteCount(void)
	{
		delete bytes_;
	}

	bool compareMAX(WORD iOne, WORD iSecond)
	{
		return iOne < iSecond;
	}

	void ByteCount::add(unsigned char _byte)
	{
		bytes_[_byte]++;
	}

	BYTE ByteCount::getMax()
	{
		BYTE popularByte = 0;
		WORD dwMax = bytes_[0];
		for (BYTE i = 1; i < BYTE_SIZE; ++i)
		{
			if (bytes_[i] > dwMax)
			{
				popularByte = i;
				dwMax = bytes_[i];
			}
		}
		return popularByte;
	}

	//void ByteCount::ShowMaxVal()
	//{
	//	std::cout << GetMax() << " ";
	//}
	//
	//void ByteCount::ShowStatictics()
	//{
	//	for (size_t i = 0; i < BYTE_SIZE; ++i)
	//		printf("%.2X - %d\r\n", i,m_vecBytes[i]);
	//}


	XorAnalyzer::XorAnalyzer(const std::string & dump_file)
		:dump_file_(dump_file)
		, hDump_(INVALID_HANDLE_VALUE)
	{
	}


	XorAnalyzer::~XorAnalyzer(void)
	{
	}

	bool XorAnalyzer::open()
	{
		return IO::open_read(hDump_, dump_file_);
	}

	void XorAnalyzer::close()
	{
		CloseHandle(hDump_);
	}

	void XorAnalyzer::Analize(const std::string & result_xor, DWORD xor_size)
	{
		HANDLE hRead = INVALID_HANDLE_VALUE;
		if (!open())
		{
			printf("Error to open file to read.\r\n");
			return;
		}

		if (xor_size <= 0)
		{
			printf("Error block size must be more than 0.\r\n");
			return;
		}
		HANDLE hWrite = INVALID_HANDLE_VALUE;
		if (!IO::create_file(hWrite, result_xor))
		{
			printf("Error to open file to write.\r\n");
			return;
		}

		ULONGLONG needMemory = NumBytesForBlock(xor_size);

		int chunks = chunksPrerBlock(needMemory);
		DWORD chunk_size = xor_size / chunks;

		DWORD buffer_size = 0;
		DWORD xor_offset = 0;
		DWORD bytesRead = 0;
		DWORD bytesWritten = 0;
		LONGLONG read_offset = 0;

		BYTE *xor_data = new BYTE[xor_size];

		for (auto nChunk = 0; nChunk < chunks; ++nChunk)
		{
			printf("Start analyzing #%d chunk of %dn\r\n", nChunk, chunks);
			xor_offset = chunk_size * nChunk;
			if (nChunk == chunks - 1)
				buffer_size = xor_size - xor_offset;
			else
				buffer_size = getChunckBufferSize(chunk_size, nChunk, xor_size);
			read_offset = xor_offset;
			ByteCount * pByteCounts = new ByteCount[buffer_size];
			BYTE * pBuffer = new BYTE[buffer_size];

			while (true)
			{
				IO::set_position(hDump_, read_offset);
				if (!IO::read_block(hDump_, pBuffer, buffer_size, bytesRead))
					break;
				if (bytesRead == 0)
					break;

				if (IO::isBlockNot00andFF(pBuffer, bytesRead))
				{
					for (DWORD nByte = 0; nByte < bytesRead; ++nByte)
						pByteCounts[nByte].add(pBuffer[nByte]);
				}

				read_offset += xor_size;
			}

			for (DWORD nByte = 0; nByte < buffer_size; ++nByte)
				xor_data[nByte + xor_offset] = pByteCounts[nByte].getMax();

			if (!IO::write_block(hWrite, xor_data + xor_offset, buffer_size, bytesWritten))
				break;
			if (bytesWritten == 0)
				break;

			delete[] pBuffer;
			delete[] pByteCounts;
		}

		delete[] xor_data;

		CloseHandle(hWrite);
		close();
	}

	DWORD XorAnalyzer::getChunckBufferSize(DWORD chunck_size, int nChunck, DWORD xor_size)
	{
		DWORD buffer_size = 0;
		DWORD xor_offset = chunck_size * nChunck;
		if ((xor_size - xor_offset) < chunck_size)
			buffer_size = xor_size - xor_offset;
		else
			buffer_size = chunck_size;
		return buffer_size;
	}
}