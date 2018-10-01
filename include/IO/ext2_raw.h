#pragma once

#include "IODevice.h"
#include "utility.h"

#include <map>
#include <utility>
#include <vector>
#include <memory>

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
//#include "factories.h"

/*
 * Structure of the super block
 */
namespace IO
{
struct ext2_super_block {
	int32_t	s_inodes_count;		/* Inodes count */
	int32_t	s_blocks_count;		/* Blocks count */
	int32_t	s_r_blocks_count;	/* Reserved blocks count */
	int32_t	s_free_blocks_count;	/* Free blocks count */
	int32_t	s_free_inodes_count;	/* Free inodes count */
	int32_t	s_first_data_block;	/* First Data Block */
	int32_t	s_log_block_size;	/* Block size */
	int32_t	s_log_frag_size;	/* Fragment size */
	int32_t	s_blocks_per_group;	/* # Blocks per group */
	int32_t	s_frags_per_group;	/* # Fragments per group */
	int32_t	s_inodes_per_group;	/* # Inodes per group */
	int32_t	s_mtime;		/* Mount time */
	int32_t	s_wtime;		/* Write time */
	int16_t	s_mnt_count;		/* Mount count */
	int16_t	s_max_mnt_count;	/* Maximal mount count */
	int16_t	s_magic;		/* Magic signature */
	int16_t	s_state;		/* File system state */
	int16_t	s_errors;		/* Behaviour when detecting errors */
	int16_t	s_minor_rev_level; 	/* minor revision level */
	int32_t	s_lastcheck;		/* time of last check */
	int32_t	s_checkinterval;	/* max. time between checks */
	int32_t	s_creator_os;		/* OS */
	int32_t	s_rev_level;		/* Revision level */
	int16_t	s_def_resuid;		/* Default uid for reserved blocks */
	int16_t	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	int32_t	s_first_ino; 		/* First non-reserved inode */
	int16_t   s_inode_size; 		/* size of inode structure */
	int16_t	s_block_group_nr; 	/* block group # of this superblock */
	int32_t	s_feature_compat; 	/* compatible feature set */
	int32_t	s_feature_incompat; 	/* incompatible feature set */
	int32_t	s_feature_ro_compat; 	/* readonly-compatible feature set */
	uint8_t	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	int32_t	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	uint8_t	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	uint8_t	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	uint16_t	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	uint8_t	s_journal_uuid[16];	/* uuid of journal superblock */
	uint32_t	s_journal_inum;		/* inode number of journal file */
	uint32_t	s_journal_dev;		/* device number of journal file */
	uint32_t	s_last_orphan;		/* start of list of inodes to delete */
	uint32_t	s_hash_seed[4];		/* HTREE hash seed */
	uint8_t	s_def_hash_version;	/* Default hash version to use */
	uint8_t	s_reserved_char_pad;
	uint16_t	s_reserved_word_pad;
	int32_t	s_default_mount_opts;
 	int32_t	s_first_meta_bg; 	/* First metablock block group */
	uint32_t	s_reserved[190];	/* Padding to the end of the block */
};

const int default_linux_block = 4096;
const int superblock_offset = 1024;
const uint16_t EXT2_SUPER_MAGIC = 0xEF53;
const int default_linux_blocks_read = 32;




struct ext2_struct_t
{
	uint32_t block_size;
	uint32_t inode_size;
};

struct HeaderInfo_t
{
	const uint8_t * header = nullptr;
	uint32_t header_size = 0;
	uint32_t header_offset = 0;
	path_string ext;
};

using HeaderInfoPtr = std::shared_ptr<HeaderInfo_t> ;

//class IOLIBRARY_EXPORT Ext2RawFactory
//	: public RawFactory
//{
//public:
//	RawAlgorithm * createRawAlgorithm(IODevicePtr device) override;
//};

	class ext2_raw
	{
		const uint32_t twelve_blocks = 12;
	private:
		IODevice * device_;
		std::vector<HeaderInfoPtr> headers_;
		uint32_t block_size_;
		uint32_t blocks_count_;
		uint64_t partition_offset_ = 0;
	public:
		ext2_raw(IODevice * device)
			: device_(device)
			, block_size_(default_linux_block)
			, blocks_count_(0)
		{

		}
		void addHeaderInfo(HeaderInfoPtr new_header)
		{
			headers_.push_back(new_header);
		}

		bool read_superblock(ext2_super_block * superblock, uint64_t offset)
		{
			if (device_->isOpen())
			{
				device_->setPosition(offset);
				Buffer buffer(default_linux_block);
				if (device_->ReadData(buffer.data, buffer.data_size))
				{
					memcpy(superblock, buffer.data + superblock_offset, sizeof(ext2_super_block));
					return true;
				}
			}
			else
				wprintf_s(L"Error. Device wasn't open.\n");

			return true;
		}

		bool isSuperblock(ext2_super_block * pSuperblock)
		{
			if (pSuperblock)
			{
				if (pSuperblock->s_magic == EXT2_SUPER_MAGIC)
					return true;
			}
			return false;
		}

		bool isTable(uint8_t * data, uint32_t data_size , uint32_t max_blocks , bool &bFullTable )
		{
			if (!data)
				return false;

			uint32_t * pValue = (uint32_t *)data;
			uint32_t * pLast = (uint32_t *)(data + data_size);
			uint32_t NullPos = 0;
			bool bFoundNull = false;
			
			if (!isLessThanValue(pValue, pLast, max_blocks, NullPos))
				return false;

			NullPos *= sizeof(uint32_t);
			if (NullPos == 0)
				return false;

			bFullTable = isFullTable(NullPos, data_size);
			if ( !bFullTable )
			{
				uint32_t * pNull = (uint32_t *)(data + NullPos);
				if (!isOnlyNulls(pNull, pLast))
					return false;
			}

			pValue = (uint32_t *)data;
			uint32_t * pEND = (uint32_t *)(data + NullPos);
			if (hasDuplicates(pValue, pEND))
				return false;

			return true;
		}

		bool isFullTable(uint32_t null_pos, uint32_t block_size)
		{
			return null_pos == block_size;
		}

		bool isLessThanValue(uint32_t *pStart, uint32_t * pLast, uint32_t value , uint32_t & null_pos)
		{
			while (pStart != pLast)
			{
				if (*pStart == 0)
					return true;

				if (*pStart > value)
					return false;

				++null_pos;
				++pStart;
			}
			return true;

		}

		bool hasDuplicates(uint32_t *pStart, uint32_t * pLast)
		{
			std::map<uint32_t, uint32_t> mapValues;
			uint32_t count = 0;
			while(pStart != pLast)
			{
				auto findIter = mapValues.find(*pStart);
				if (findIter != mapValues.end())
					return true;

				mapValues.insert(std::make_pair(*pStart, *pStart));

				++pStart;
				++count;
			}
			return false;
		}

		bool isOnlyNulls(uint32_t * pStart , uint32_t * pLast)
		{
			while (pStart != pLast)
			{
				if (*pStart != 0)
					return false;
				++pStart;
			}
			return true;
		}

		HeaderInfoPtr isHeader(const uint8_t * data)
		{
			for (auto header : this->headers_)
			{
				if (memcmp(header->header, data + header->header_offset, header->header_size) == 0)
					return header;
			}
			return nullptr;
		}

		HeaderInfoPtr findHeader(const uint64_t start_offset, uint64_t &header_offset)
		{
			if (!this->device_)
				return nullptr;
			header_offset = start_offset;
			Buffer buffer(default_block_size);
			uint32_t bytesRead = 0;

			while (header_offset < device_->Size())
			{
				device_->setPosition(header_offset);
				bytesRead = this->device_->ReadData(buffer.data, buffer.data_size);
				if (bytesRead == 0)
				{
					wprintf_s(L"Error read data FindHeader\n");
					return nullptr;
				}
				for (uint32_t iBlock = 0; iBlock < bytesRead; iBlock += block_size_)
				{
					auto header_info = isHeader(buffer.data + iBlock);
					if (header_info)
					{
						header_offset += iBlock;
						return header_info;
					}
				}
				header_offset += buffer.data_size;
			}
			return nullptr;
		}

		uint64_t saveFile(const uint64_t header_offset, path_string file_name)
		{

			uint32_t bytes_read = 0;
			uint32_t bytes_written = 0;
			uint32_t first_blocks_size = block_size_*twelve_blocks;
			uint64_t table_offset = header_offset +(uint64_t)first_blocks_size;

			uint64_t offset = header_offset;
			uint32_t buffer_size = block_size_ * default_linux_blocks_read;
			Buffer buffer(buffer_size);


			Buffer table_buffer(block_size_);
			
			device_->setPosition(table_offset);
			bytes_read = device_->ReadData(table_buffer.data, table_buffer.data_size);
			if (bytes_read != table_buffer.data_size)
				return ERROR_READ_FILE;

			bool bFullTable = false;
			if (!isTable(table_buffer.data, table_buffer.data_size, blocks_count_, bFullTable))
				return ERROR_RESULT;

			File write_file(file_name);
			if (!write_file.Open(OpenMode::Create))
				return ERROR_OPEN_FILE;

			if (!copyTo(device_, offset, &write_file, 0, first_blocks_size))
				return ERROR_READ_FILE;


			saveTable(&table_buffer, device_, &write_file);
			
			uint64_t result_offset = header_offset;

			// read Double Indirect Blocks
			if (bFullTable)
			{
				result_offset = table_offset;
				uint32_t * pLastBlock = (uint32_t*)(table_buffer.data + table_buffer.data_size - sizeof(uint32_t));
				uint32_t dib_pointer = (*pLastBlock + 1);
				table_offset = (uint64_t)dib_pointer  *  (uint64_t)block_size_ + partition_offset_;

				bytes_read = device_->ReadData(table_buffer.data, table_buffer.data_size);
				if (bytes_read != table_buffer.data_size)
					return ERROR_READ_FILE;
				
				// is DIB table
				bFullTable = false;
				if (isTable(table_buffer.data, table_buffer.data_size, blocks_count_, bFullTable))
				{
					uint32_t * pDIB_pointetr = (uint32_t*)table_buffer.data;
					for (uint32_t iPtr = 0; iPtr < table_buffer.data_size; iPtr += sizeof(uint32_t))
					{
						if ( *pDIB_pointetr == 0 )
							break;
						uint64_t dib_table_offset = (uint64_t)(*pDIB_pointetr) * (uint64_t)block_size_ + partition_offset_;
						Buffer dib_table(block_size_);
						device_->setPosition(dib_table_offset);
						bytes_read = device_->ReadData(dib_table.data, dib_table.data_size);
						if (bytes_read != dib_table.data_size)
							return ERROR_READ_FILE;

						bool dib_fulltable = false;
						if (isTable(dib_table.data, dib_table.data_size, blocks_count_, dib_fulltable))
							saveTable(&dib_table, device_, &write_file);


						++pDIB_pointetr;
					}
				}


			}
			return result_offset;

		}
		void saveTable(Buffer * buffer, IODevice * source, IODevice *target)
		{
			uint32_t * pBlockOffset = (uint32_t*)buffer->data;
			uint64_t block_offset = 0;
			for (uint32_t iBlock = 0; iBlock < buffer->data_size / sizeof(uint32_t); ++iBlock)
			{
				if (*pBlockOffset == 0)
					break;

				block_offset = (uint64_t)*pBlockOffset * (uint64_t)block_size_;
				block_offset += +partition_offset_;
				if (!copyTo(source, block_offset, target, 0, block_size_))
					return;
				++pBlockOffset;
			}

		}

		bool copyTo(IODevice * source, uint64_t source_offset, IODevice * target, uint64_t target_offset, uint32_t copy_size)
		{
			uint32_t bytes_read = 0;
			Buffer buffer(copy_size);
			source->setPosition(source_offset );
			bytes_read = device_->ReadData(buffer.data, buffer.data_size);
			if (bytes_read != copy_size)
				return false;


			uint32_t bytes_written = 0;
			bytes_written = target->WriteData(buffer.data, buffer.data_size);
			if (bytes_written != copy_size)
				return false;	
			return true;
		}
		void execute(const path_string & target_folder)
		{
			if (!device_->Open(OpenMode::OpenRead))
			{
				wprintf_s(L"Error open source device\n");
				return;
			}


			//auto psd_header_info = std::make_shared<HeaderInfo_t>();
			//psd_header_info->header = Signatures::psd_header;
			//psd_header_info->header_size = Signatures::psd_header_size;
			//psd_header_info->ext = L".psd";
			//this->addHeaderInfo(psd_header_info);

			//auto cdr_header_info = std::make_shared<HeaderInfo_t>();
			//cdr_header_info->header = Signatures::cdr_header;
			//cdr_header_info->header_size = Signatures::cdr_header_size;
			//cdr_header_info->ext = L".cdr";
			//this->addHeaderInfo(cdr_header_info);

			//auto jpg_header_info = std::make_shared<HeaderInfo_t>();
			//jpg_header_info->header = Signatures::jpg_header;
			//jpg_header_info->header_size = Signatures::jpg_header_size;
			//jpg_header_info->ext = L".jpg";
			//this->addHeaderInfo(jpg_header_info);

			//auto tif_header_info = std::make_shared<HeaderInfo_t>();
			//tif_header_info->header = Signatures::tif_header;
			//tif_header_info->header_size = Signatures::tif_header_size;
			//tif_header_info->ext = L".tif";
			//this->addHeaderInfo(tif_header_info);

			//auto tif2_header_info = std::make_shared<HeaderInfo_t>();
			//tif2_header_info->header = Signatures::tif2_header;
			//tif2_header_info->header_size = Signatures::tif2_header_size;
			//tif2_header_info->ext = L".tif";
			//this->addHeaderInfo(tif2_header_info);

			//const uint8_t mpg_header[] = { 0x00 , 0x00 , 0x01 , 0xBA };
			//const uint32_t mpg_header_size = SIZEOF_ARRAY(mpg_header);

			//auto mpg_header_info = std::make_shared<HeaderInfo_t>();
			//mpg_header_info->header = mpg_header;
			//mpg_header_info->header_size = mpg_header_size;
			//mpg_header_info->ext = L".mpg";
			//this->addHeaderInfo(mpg_header_info);

			//const uint8_t qt_ftyp[] = { 0x66 , 0x74 , 0x79 , 0x70 };
			//const uint32_t qt_ftyp_size = SIZEOF_ARRAY(qt_ftyp);
			//const uint32_t qt_ftyp_offset = 4;

			//auto qt_header_info = std::make_shared<HeaderInfo_t>();
			//qt_header_info->header = qt_ftyp;
			//qt_header_info->header_size = qt_ftyp_size;
			//qt_header_info->header_offset = qt_ftyp_offset;
			//qt_header_info->ext = L".mov";
			//this->addHeaderInfo(qt_header_info);

			//const uint8_t avi_header[] = { 0x52 , 0x49 , 0x46 , 0x46 };
			//const uint32_t avi_header_size = SIZEOF_ARRAY(avi_header);

			//auto avi_header_info = std::make_shared<HeaderInfo_t>();
			//avi_header_info->header = avi_header;
			//avi_header_info->header_size = avi_header_size;
			//avi_header_info->ext = L".avi";
			//this->addHeaderInfo(avi_header_info);




			//auto ai_header_info = std::make_shared<HeaderInfo_t>();
			//ai_header_info->header = Signatures::ai_header;
			//ai_header_info->header_size = Signatures::ai_header_size;
			//ai_header_info->ext = L".ai";
			//this->addHeaderInfo(ai_header_info);

			auto office_2007_header_info = std::make_shared<HeaderInfo_t>();
			office_2007_header_info->header = Signatures::office_2007_header;
			office_2007_header_info->header_size = Signatures::office_2007_header_size;
			office_2007_header_info->ext = L".zip";
			this->addHeaderInfo(office_2007_header_info);

			auto office_2003_header_info = std::make_shared<HeaderInfo_t>();
			office_2003_header_info->header = Signatures::office_2003_header;
			office_2003_header_info->header_size = Signatures::office_2003_header_size;
			office_2003_header_info->ext = L".msdoc";
			this->addHeaderInfo(office_2003_header_info);

			//auto pdf_header_info = std::make_shared<HeaderInfo_t>();
			//pdf_header_info->header = Signatures::pdf_header;
			//pdf_header_info->header_size = Signatures::pdf_header_size;
			//pdf_header_info->ext = L".pdf";
			//this->addHeaderInfo(pdf_header_info);

			ext2_super_block super_block = { 0 };
			if (!read_superblock(&super_block, 0))
				return;

			if (isSuperblock(&super_block))
			{
				wprintf_s(L"Error super_block\n");
				return;
			}
			blocks_count_ = super_block.s_blocks_count;
			//blocks_count_ = 0xE8A0DAE;
			// super_block offset = 0x40001000
			uint64_t offset = 0;
			partition_offset_ = 0;
			uint64_t header_offset = 0;
			uint64_t tmp_offset = 0;
			uint32_t counter = 0;
			while (true)
			{
				header_offset = 0;
				auto header_info = findHeader(offset, header_offset);
				if ( !header_info)
					break;

				offset = header_offset;

				auto new_folder = addBackSlash(target_folder) + header_info->ext.substr(1, header_info->ext.length() -1);
				if (!fs::exists(new_folder))
					fs::create_directory(new_folder);

				auto target_file = toFullPath(new_folder, counter++, header_info->ext);
				tmp_offset = saveFile(header_offset, target_file);
				if (tmp_offset == ERROR_RESULT)
				{
					wprintf_s(L"Not found table\n");
				}
				else
					if (tmp_offset == ERROR_READ_FILE)
						wprintf_s(L"Error read file\n");
					else
					{
						offset = tmp_offset;
					}



				offset += block_size_;

			}
		}
		

	};
};