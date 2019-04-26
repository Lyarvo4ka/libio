#include <QtCore/QCoreApplication>

#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include <QDebug>
#include <QList>



#include <boost/lexical_cast.hpp>

#include <iostream>

#include "../JsonReader/JsonReader.h"


#include "IO\AbstractRaw.h"
#include "IO\Factories.h"
#include "IO\RawMTS.h"
#include "IO\RawMPEG.h"
#include "IO\QuickTime.h"
#include "IO\RawRIFF.h"
#include "IO\RawMXF.h"
#include "IO\RawIMD.h"
#include "IO\keychainraw.h"
#include "IO\PCKRaw.h"
#include "IO\ZS2Raw.h"
#include "IO\ZoomH6Raw.h"
#include "IO\RawPLN.h"
#include "IO\GoPro.h"
#include "IO\RawCDW.h"
#include "IO\FlpRaw.h"
#include "IO/djidrone.h"


const int param_count = 6;
const int offset_param = 1;
const int offset_value = 2;
const int disk_file_param = 3;
const int source_value = 4;
const int target_value = 5;

const std::string d_str = "-d";
const std::string f_str = "-f";
const std::string offset_str = "-offset";

#include <memory>

void initVideoFactoryManager(IO::RawFactoryManager & factory_manager)
{
	factory_manager.Register("mts", std::make_unique<IO::RawMTSFactory>());
	factory_manager.Register("mpeg", std::make_unique<IO::RawMPEGFactory>());
	factory_manager.Register("quicktime", std::make_unique<IO::QuickTimeRawFactory>());
	factory_manager.Register("avi", std::make_unique<IO::RawFIFFFactory>());

}
void initAudioFactoryManager(IO::RawFactoryManager & factory_manager)
{
	factory_manager.Register("wave", std::make_unique<IO::RawFIFFFactory>());
}

void initKeysFactoryManager(IO::RawFactoryManager & factory_manager)
{
	factory_manager.Register("zs2_514C", std::make_unique<IO::ZS2RawFactory>());
	//factory_manager.Register("zs2_3082", std::make_unique<IO::ZS2Raw_3082Factory>());
	factory_manager.Register("pck", std::make_unique<IO::PCKRawFactory>());
}

void initFactoryMananger(IO::RawFactoryManager & factory_manager)
{
	//initVideoFactoryManager(factory_manager);

	//factory_manager.Register("flp", std::make_unique<IO::RawFLPFactory>());
	factory_manager.Register("DjiDrone", std::make_unique<DjiDroneRawFactory>());


	//initKeysFactoryManager(factory_manager);
	//factory_manager.Register("qt_fragment", std::make_unique<IO::QTFragmentRawFactory>());
	//initAudioFactoryManager(factory_manager);
	//factory_manager.Register("go_pro", std::make_unique<IO::GoProRawFactory>());

	//factory_manager.Register("pln", std::make_unique<IO::PLNRawFactory>());

	//factory_manager.Register("keychain-db", std::make_unique<IO::KeychainRawFactory>());

	//factory_manager.Register("mxf", std::make_unique<IO::RawMXFFactory>());

	//factory_manager.Register("go_pro", std::make_unique<IO::GoProRawFactory>());



	//factory_manager.Register("BlackVue", std::make_unique<IO::BlackVue_QtRawFactory>());
	//factory_manager.Register("mx7", std::make_unique<IO::RawFIFFFactory>());

	//factory_manager.Register("r3d", std::make_unique<IO::StandartRawFactory>());
	//factory_manager.Register("ZOOMHandyRecorder", std::make_unique<IO::RawZOOMHandyRecorder>());
	
	//factory_manager.Register("imd", std::make_unique<IO::RawIMDFactory>());
	//factory_manager.Register("cdw", std::make_unique<IO::RawCWDFactory>());

	//factory_manager.Register("Canon80D", std::make_unique<IO::Canon80D_FragmentRawFactory>());


}

#include "zlib.h"



int main(int argc, char *argv[])
{

	QCoreApplication a(argc, argv);
	IO::IODevicePtr src_device = nullptr;
	
	// 1.-offset 2.offset_val 3.(-d -f) 4.path 5.target
	if (argc == param_count)
	{
		uint64_t start_offset = 0;
		std::string offset_txt(argv[offset_param]);
		if (offset_txt.compare(offset_str) == 0)
		{
			start_offset = boost::lexical_cast<uint64_t>(argv[offset_value]);
			qInfo() << "offset : " << start_offset <<"(sectors)";
		}


		std::string disk_file_string(argv[disk_file_param]);
		if (disk_file_string.compare(d_str) == 0)
		{
			auto drive_number = boost::lexical_cast<uint32_t>(argv[source_value]);

			auto drive_list = IO::ReadPhysicalDrives();
			auto physical_drive = drive_list.find_by_number(drive_number);
			if (!physical_drive)
			{
				qInfo() << "Error open physical drive #" << drive_number;
				return -1;
			}
			start_offset *= physical_drive->getBytesPerSector();
			if (physical_drive)
			{
				qInfo() << "You selected";
				qInfo() << "Number : " << drive_number;
				qInfo() << "Name :" << physical_drive->getDriveName().c_str();
				qInfo() << "Serial number : " << physical_drive->getSerialNumber().c_str();
				qInfo() << "Size : " << physical_drive->getSize() << "(bytes)";
			}
			src_device = std::make_shared<IO::DiskDevice>(physical_drive);
		}
		else if (disk_file_string.compare(f_str) == 0)
		{
			std::string src_path = argv[source_value];
			src_device = IO::makeFilePtr(IO::path_string(src_path.begin(), src_path.end()));
			start_offset *= default_sector_size;
		}

		if (!src_device->Open(IO::OpenMode::OpenRead))
		{
			qInfo() << "Error open source device.";
			return -1;
		}

		std::string targer_path = argv[target_value];
		IO::path_string target_folder(targer_path.begin(), targer_path.end());

		if (!src_device)
			return -1;

		//////////////////////////////////////////////////////////////////////////
		QList<JsonFileStruct> listFileStruct;

		//QString json_file = R"(d:\develop\libio\RawRecoveryConsole\base\video\video.json)";
		QString json_file = "DjiDrone.json";
		QFile file(json_file);
		if (!file.open(QIODevice::ReadOnly))
		{
			qInfo() << "Error to open file. \"" << file.fileName() << "\"";
			return -1;
		}

		auto json_str = file.readAll();
		ReadJsonFIle(json_str, listFileStruct);
		if ( listFileStruct.empty())
		{
			qInfo() << "Error to read" << file.fileName() << "file. Wrong syntax.";
			return -1;
		}

		IO::HeaderBase::Ptr headerBase = std::make_shared<IO::HeaderBase>();
		for (auto theFileStruct : listFileStruct)
			headerBase->addFileFormat(toFileStruct(theFileStruct));

		IO::RawFactoryManager factory_manager;
		initFactoryMananger(factory_manager);
		//factory_manager.Register("qt_canon", std::make_unique<IO::Canon80D_FragmentRawFactory>());

		IO::SignatureFinder signatureFinder(src_device, headerBase);

		//start_offset = 0x11DE3200;
		uint64_t header_offset = 0;
		uint32_t counter = 0;
		//const IO::path_string dst_folder = L"d:\\incoming\\43944\\result\\";
		while (start_offset < src_device->Size())
		{
			auto file_struct = signatureFinder.findHeader(start_offset, header_offset);
			if (!file_struct)
			{
				qInfo() << endl << endl << endl << "No more signatures found. Press any key to exit.";
				break;
			}
			qInfo() << "Found signature for [" << file_struct->getName().c_str() << "] file."; 
			qInfo() << "Offset : " << header_offset << "(bytes)";

			start_offset = header_offset;
			/*
				if (type ==special) find in other base factory			
			*/

			//IO::ZoomH6Raw zoomH6Raw(src_device);
			//auto bytesWritten = zoomH6Raw.Execute(header_offset, target_folder);
			//if (bytesWritten == 0)
			//	break;
			//start_offset += default_sector_size;



			

 			auto raw_factory = factory_manager.Lookup(file_struct->getName());
			IO::RawAlgorithm * raw_algorithm = nullptr;
			if (!raw_factory)
			{
				IO::StandartRaw * standard_raw = new IO::StandartRaw(src_device);
				standard_raw->setMaxFileSize(file_struct->getMaxFileSize());
				standard_raw->setFooter(file_struct->getFooter(), file_struct->getFooterTailEndSize());
				//standard_raw->setFooterOffsetSearchBlock(4, 4096);

				raw_algorithm = standard_raw;
				
			}
			else
			{
				raw_algorithm = raw_factory->createRawAlgorithm(src_device);
				IO::StandartRaw * tmpPtr = dynamic_cast<IO::StandartRaw *>(raw_algorithm);
				if (tmpPtr)
					tmpPtr->setMaxFileSize(file_struct->getMaxFileSize());
			}
				
				if (raw_algorithm->Specify(header_offset))
				{
					auto target_file = IO::offsetToPath(target_folder, header_offset, file_struct->getExtension(), default_sector_size);
					auto dst_file = IO::makeFilePtr(target_file);
					if (dst_file->Open(IO::OpenMode::Create))
					{
						auto target_size = raw_algorithm->SaveRawFile(*dst_file, header_offset);
						
						if ( target_size == 0)
						{
							qInfo() << "Error to save file." ;
							//break;

						}
						auto dst_size = dst_file->Size();
						dst_file->Close();
						qInfo() << "Successfully saved " << target_size << "(bytes)" << endl << endl;

						uint64_t jump_size = default_sector_size;

						if ( raw_algorithm->Verify(target_file) )
						{
							target_size /= default_sector_size;
							target_size *= default_sector_size;
							//////////////////////////////////////////////////////////////////////////
							jump_size = target_size;
						}
						else
						{
							// remove file
							IO::path_string new_fileName = target_file + L".bad_file";
							boost::filesystem::rename(target_file, new_fileName);
							qInfo() << "Renamed to .bad_file";
							//{
							//	qInfo() << "File" << target_file.c_str() << "was removed." << endl;
							//}
							//else
							//	qInfo() << "File" << target_file.c_str() << "Error to delete." << endl;


						}
						//if (jump_size == 0)
							jump_size = default_sector_size;
						start_offset = header_offset + jump_size;

					}
					else
					{
						qInfo() << "Error to create target file." << QString::fromStdWString(target_file);
						qInfo() << "Exit.";
						break;
					}
					
							

				}
				else
				{
					qInfo() << "Not specified for " << QString::fromStdString(file_struct->getName()) << "continue search for other signatures."<<endl;
					start_offset += default_sector_size;
				}
				if ( raw_algorithm)
					delete raw_algorithm;

				

			}



		

			qInfo() << "Finished.";
	}
	else
		qInfo() << "Wrong params";
	return a.exec();
}
