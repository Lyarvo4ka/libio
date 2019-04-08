#pragma once

#include "IO/constants.h"
#include "IO/dataarray.h"
#include "IO/IODevice.h"
#include "tiffio.h"
#include <iostream>

namespace IO
{
	template <typename T>
	struct DateType
	{
		T year;
		T month;
		T day;
		T hour;
		T min;
		T sec;
	};

	class Analyzer
	{
	public:
		virtual void analyze(const path_string & filePath) = 0;

		//bool isCorupted() const ;
		//bool hasName() const;
		//bool hasTime() const;
		//path_string getExtension() const;
	};

	const uint32_t RGB_pixelSize = 3;

#pragma pack( push, 1)
	struct Pixel
	{
		int8_t R;
		int8_t G;
		int8_t B;

		Pixel opertor -(const Pixel & val1 , const Pixel & val2 )
		{
			Pixel result;
			result.R = val1.R - val2.R;
			resutl.G = val1.G - val2.G;
			result.B = val1.B - val2.B;
			return result;
		}
	};
#pragma pack(pop)

	class TiffAnalyzer
		: public Analyzer
	{
		uint32_t pixelSize_ = RGB_pixelSize;
		uint32_t delta_ = 0;
		bool bCorrupted_ = false;
	public:
		bool isCorupted() const
		{
			return bCorrupted_;
		}
		void AnalyzeTiffData(const IO::DataArray & tiff_data, uint32_t width, uint32_t height , uint32_t nLine) 
		{
			const uint32_t numBits = 3;
			const uint32_t lineSize = width * numBits;

			IO::File deltaFile(L"delta.txt");
			deltaFile.OpenCreate();
			int8_t bitsArray[numBits];

			uint64_t delta_sum = 0;

			int delta_mid = 0;
			const uint32_t lastLine = (height -1)* lineSize + nLine * numBits;

			//Pixel * pCurrent = (Pixel *)tiff)data.data();

			int8_t * pCur = (int8_t *)(tiff_data.data() + lastLine);
			for (uint32_t iHeight = 1; iHeight < height - 1; ++iHeight)
			{
				int8_t * pNext = (int8_t *)(tiff_data.data() + lastLine - iHeight * lineSize);
				std::string writeStr;
				delta_mid = 0;
				for (uint32_t iBit = 0; iBit < numBits; ++iBit)
				{
					int8_t delta_val = pNext[iBit] - pCur[iBit];
					int8_t abs_delta = std::abs(delta_val);
					if (abs_delta < 0)
					{
						int k = 1;
						k = 2;
					}
					bitsArray[iBit] = abs_delta;
					//writeStr += std::to_string(bitsArray[iBit]) + " ";
					delta_mid += bitsArray[iBit];
				}

				delta_mid /= numBits;
				delta_sum += delta_mid;
				if (delta_mid > 55)
				{
					int k = 1;
					k = 2;
					bCorrupted_ = true;

				}
				pCur = pNext;
				writeStr += /*" mid =" + */std::to_string(delta_mid) + "\r\n";
				deltaFile.WriteText(writeStr);
			}
			if (delta_sum == 0)
				bCorrupted_ = true;
			return ;
		}
		void analyze(const path_string & filePath) override
		{
			

			TIFF *tif = TIFFOpenW(filePath.c_str(), "r");
			if (!tif)
			{
				std::cout << "Error open tiff" << std::endl;
				return;
			}

			uint32_t width = 0;
			uint32_t height = 0;
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

			uint32_t npixels = width * height;
			auto buff_size = npixels * sizeof(uint32_t);
			auto raster = (uint32_t *)_TIFFmalloc(buff_size);
			auto result = TIFFReadRGBAImage(tif, width, height, raster, 0);

			if (result == 0)
			{
				std::cout << "Error" << std::endl;
				return;
			}

			IO::DataArray write_buff(npixels * pixelSize_);

			for (uint32_t iPixel = 0; iPixel < npixels; ++iPixel)
			{
				memcpy(write_buff.data() + iPixel * pixelSize_ , raster + iPixel, pixelSize_);
			}

			BITMAPINFOHEADER bfh = BITMAPINFOHEADER();
			//BITMAPINFO bi;
			bfh.biSize = sizeof(BITMAPINFOHEADER);
			bfh.biHeight = height;
			bfh.biWidth = width;
			bfh.biBitCount = 24;
			bfh.biPlanes = 1;
			bfh.biCompression = BI_RGB;


			const char BM_TEXT[] = "BM";
			BITMAPFILEHEADER bm_header = BITMAPFILEHEADER();
			ZeroMemory(&bm_header, sizeof(BITMAPFILEHEADER));
			memcpy(&bm_header.bfType, BM_TEXT, 2);
			bm_header.bfSize = sizeof(BITMAPFILEHEADER) + bfh.biSize + buff_size;

			IO::File bmp_file(L"1.bmp");
			bmp_file.OpenCreate();
			bmp_file.WriteData((IO::ByteArray)&bm_header, sizeof(BITMAPFILEHEADER));
			bmp_file.WriteData((IO::ByteArray)&bfh, bfh.biSize);
			bmp_file.WriteData(write_buff.data(), write_buff.size());
			bmp_file.Close();


			AnalyzeTiffData(write_buff, width, height , 0);
			if (!isCorupted())
				AnalyzeTiffData(write_buff, width, height, height-1);
			_TIFFfree(raster);
			TIFFClose(tif);
		}
	};


}