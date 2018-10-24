#pragma once

#include "iodevice.h"
#include "AbstractRaw.h"

namespace IO
{

	const uint16_t EXTENT_HEADER_MAGIC = (uint16_t)0xF30A;
	
	const uint16_t INODE_SIZE = 256;

#pragma pack(push)
#pragma pack(1)

	typedef struct _ext4_extent {
		uint32_t block;		//	First file block number that this extent covers.
		uint16_t len;
		uint16_t start_hi;	//	Upper 16-bits of the block number to which this extent points.
		uint32_t start_lo;	//	Lower 32-bits of the block number to which this extent points.
		uint64_t PysicalBlock(void) { return ((uint64_t)start_lo) | (((uint64_t)start_hi) << 32); }
	} ext4_extent;

	typedef struct _ext4_extent_idx {
		uint32_t block;		//	This index node covers file blocks from 'block' onward.
		uint32_t leaf_lo;		//	Lower 32-bits of the block number of the extent node that is the next level lower in the tree. 
							//	The tree node pointed to can be either another internal node or a leaf node, described below.
		uint16_t leaf_hi;		//	Upper 16-bits of the previous field.
		uint16_t unused;
		uint64_t PysicalBlock(void) { return ((uint64_t)leaf_lo) | (((uint64_t)leaf_hi) << 32); }
	} ext4_extent_idx;


	typedef struct _ext4_extent_header {
		uint16_t magic;
		uint16_t entries;
		uint16_t max;
		uint16_t depth;
		uint32_t generation;
	} ext4_extent_header;

	typedef struct _EXTENT_BLOCK {
		ext4_extent_header header;
		union {
			ext4_extent extent[1];
			ext4_extent_idx extent_index[1];
		};
	} EXTENT_BLOCK;


	struct ext4_inode
	{
		uint8_t skip;
		EXTENT_BLOCK extent_block;
	};

	class ext4_raw
		: public SpecialAlgorithm
	{
		IODevicePtr device_;
		uint64_t volume_offset_ = 0;
		uint32_t block_size_ = 4096;
		uint16_t max_extents_in_block_;
	public:
		ext4_raw(IODevicePtr device)
			: device_(device)
		{
			max_extents_in_block_ = (block_size_ - sizeof(ext4_extent_header)) / sizeof(ext4_extent);
		}
		uint64_t Execute(const uint64_t inode_offset, const path_string target_folder) override
		{
			if (!device_->isOpen())
				return 0;

			//DataArray inode(256);
			//device_->setPosition(inode_offset);
			//device_->ReadData(inode.data(), inode.size());
			//EXTENT_BLOCK *pExtBlock = (EXTENT_BLOCK*)(inode.data());

			//auto ext_offset = pExtBlock->extent_index->PysicalBlock();
			
			//DataArray next_inode(4096);
			//device_->setPosition(inode_offset);
			//device_->ReadData(next_inode.data(), next_inode.size());
			//EXTENT_BLOCK *pNext = (EXTENT_BLOCK*)(next_inode.data());

			//int k = 1;
			//k = 2;
			auto ext_offset = inode_offset / 4096;
			uint64_t inode_block = ext_offset/* / 4096*/;
			File target_file(target_folder);
			target_file.OpenCreate();


			SaveToFile(inode_block , target_file);
			//auto inode = read_inode(inode_offset);
			//ext4_inode * pInode = (ext4_inode*)inode.data();
			//if (pInode->extent_block.header.magic == EXTENT_HEADER_MAGIC)
			//{
			//}
			return 0;
		}
		DataArray read_inode(const uint64_t inode_offset)
		{
			DataArray inode(INODE_SIZE);
			device_->setPosition(inode_offset);
			device_->ReadData(inode.data(), inode.size());
			return inode;
		}
    void search_extends(uint64_t block_start)
    {
		//const uint32_t count = 16;
		//const uint32_t buff_size = count * block_size_;
		//DataArray buff(buff_size);
		//uint64_t offset = (uint64_t)(block_start * block_size_);
		//uint64_t src_size = device_->Size();
		//uint32_t to_read = 0;
		//uint32_t bytes_read = 0;

		//while (offset < device_->Size())
		//{
		//	to_read = calcBlockSize(offset, src_size, buff_size);
		//	device_->setPosition(offset);
		//	device_->ReadData(buff.data(), to_read);

		//	for (uint32_t i = 0; i < to_read, i += block_size_)
		//	{
		//		EXTENT_BLOCK * p_extent = (EXTENT_BLOCK *)(buff.data() + i);

		//	}


		//	offset += to_read;
		//}


    }
		uint64_t SaveToFile(const uint64_t block_num, File &target_file)
		{
			if (!target_file.isOpen())
				return 0;

			DataArray buffer(block_size_);
			EXTENT_BLOCK *extent_block = (EXTENT_BLOCK *)buffer.data();

			uint64_t extent_offset = volume_offset_ + block_num * block_size_;
			device_->setPosition(extent_offset);
			device_->ReadData(buffer.data(),buffer.size());
			
			if ((extent_block->header.magic != EXTENT_HEADER_MAGIC) ||
				(extent_block->header.max != max_extents_in_block_) ||
				(extent_block->header.entries > max_extents_in_block_)) {
				return 0;
			}

			//std::vector<BYTE> data_buff;
			DataArray data_array(default_block_size);
			uint64_t offset = 0;
			uint32_t size = 0;
			if (extent_block->header.depth == 0) {
				for (int i = 0; i < extent_block->header.entries; i++) {
					offset = volume_offset_ + extent_block->extent[i].PysicalBlock() * block_size_;

					size = (extent_block->extent[i].len <= 0x8000) 
						? (extent_block->extent[i].len * block_size_) 
						: ((extent_block->extent[i].len - 0x8000) * block_size_);

					if (data_array.size() < size) {
						data_array.resize(size);
					}

					if (extent_block->extent[i].len <= 0x8000) 
					{
						device_->setPosition(offset);
						device_->ReadData(data_array.data(), size);
					}
					else {
						memset(data_array.data(), 0x00, size);
					}

					uint64_t target_offset = (uint64_t)extent_block->extent[i].block * block_size_;
					target_file.setPosition(target_offset);
					target_file.WriteData(data_array.data(), size);
				}
			}
			else {
				for (int i = 0; i < extent_block->header.entries; i++) {
					SaveToFile(extent_block->extent_index[i].PysicalBlock(), target_file);
					int x = 0;
				}
			}
			return 0;
		}

	//	uint64_t SaveRawFile(uint64_t iNode_offset)
	};

#pragma pack(pop)
}