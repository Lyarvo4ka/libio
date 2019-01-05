#include "IO/SignatureTest.h"
#include "IO/Finder.h"

namespace IO
{


	bool testTiffSignature(const IO::path_string & filePath)
	{
		const uint8_t tif_4949[] = { 0x49, 0x49 };
		const uint8_t tif_4D4D[] = { 0x4D, 0x4D };
		const uint32_t tif_sing_size = 2;

		IO::File tiff_file(filePath);
		tiff_file.OpenRead();

		DataArray buff(tif_sing_size);

		if (tiff_file.Size() < tif_sing_size)
			return false;
		
		tiff_file.ReadData(buff);

		if (memcmp(buff.data(), tif_4949, tif_sing_size) != 0)
			if (memcmp(buff.data(), tif_4D4D, tif_sing_size) != 0)
				return false;

		return true;
	}

	void Signture_Testing(const path_string & folder)
	{
		Finder finder;
		finder.add_extension(L".tif");

		finder.FindFiles(folder);
		auto fileList = finder.getFiles();

		for (auto & theFile : fileList)
		{
			try {
				std::wcout << theFile;
				//IO::testHeaderToBadSectorKeyword(theFile);
				if (!testTiffSignature(theFile))
					fs::rename(theFile, theFile + L".bad_file");

				std::cout << std::endl;
			}
			catch (IO::Error::IOErrorException & ex)
			{
				const char* text = ex.what();
				std::cout << " Cougth exception " << text;

			}
			catch (fs::filesystem_error ex)
			{
				const char* text = ex.what();
				std::cout << " Cougth exception " << text;
			}


		}
	}
}