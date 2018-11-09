#pragma once

#include "FileSystemDLL.h"

#include <memory>

namespace FileSystem
{
	class FSExport Cluster
	{
	public:
		Cluster(const DWORD size, const DWORD number = 0);
		~Cluster();
		void setNumber(const DWORD number);
		BYTE * data() ;
		DWORD size() const;
		DWORD number() const;
	private:
		DWORD size_;
		DWORD number_;
		BYTE * data_;
	};

	class FSExport IVirualReader
	{
	public:
		// Destructor
		virtual ~IVirualReader() {} /*= 0*/;

		// Opening file system...
		// from offset that before setting, it's boot sector.
		virtual bool Open() = 0;

		// Return status to ready this reader.
		virtual bool isReady() const = 0;

		//	read sector:
		//	"data" allocated buffer in sector size,
		//	"sector_number" position sector
		virtual bool ReadSector(BYTE * sector_data, LONGLONG sector_number) = 0;

		virtual bool ReadSectors(BYTE * sector_data, LONGLONG sector_number , const DWORD count_sectors) = 0;

	};

	class AbstractReader
	{
	public:
		virtual ~AbstractReader() = 0;
		virtual bool Open() = 0;
		virtual bool isOpen() = 0;
		virtual bool Read() = 0;
	};

	typedef std::shared_ptr<IVirualReader> VirtualReader;
////////////////////// CSectorReader /////////////////////
	class FSExport CSectorReader
		: public IVirualReader
	{
	public:
		CSectorReader(const AbstractReader & reader, const DWORD sector_size);
		virtual ~CSectorReader();

		virtual bool Open() ;
		virtual bool isReady() const;
		virtual bool ReadSector(BYTE * sector_data, LONGLONG sector_number) ;
		virtual bool ReadSectors(BYTE * sector_data, LONGLONG sector_number , const DWORD count_sectors) ;

		void setOffset(LONGLONG offset);

	private:
		AbstractReader Reader_;
		DWORD sector_size_;
		LONGLONG position_;
		//string error_;

	};
	typedef std::shared_ptr<CSectorReader> SectorReader; 

	//class FSExport FatReader
	//{
	//public:
	//	explicit FatReader(IReader * pReader);
	//	virtual ~FatReader();

	//	virtual bool ReadSector(BYTE * pSector, LONGLONG SectorNumber);
	//	virtual DWORD ReadSectors(BYTE * pSectorData, LONGLONG SectorNumber , const DWORD SectorCounts);

	//	void isReady() const;
	//	DWORD Read(BYTE * pReadBuffer, const DWORD ReadSize, const LONGLONG Position);
	//	DWORD Read(CBuffer * pBuffer, const LONGLONG Position);
	//	bool read_cluster(Cluster * pCluster, const DWORD sector_from);

	//	void SetSectorSize(const DWORD BytesPerSector);
	//	void SetReservedSectors(const DWORD ReservedSectors);
	//	void SetDataOffset(const DWORD data_offset);
	//private:
	//	IReader *Reader_;
	//	DWORD BytesPerSector_;
	//	DWORD ReservSectors_;
	//	DWORD DataOffset_;
	//};



}
