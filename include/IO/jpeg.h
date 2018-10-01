#pragma once

#include "jpeglib.h"
#include "AbstractRaw.h"
#include <exception>
#include <Wingdi.h>
#include "Finder.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace IO
{
	class my_exception
		: public std::exception
	{
	public:
		my_exception(std::string && expection_text) noexcept
			: text_(std::move(expection_text))
		{}
		my_exception(const std::string & expection_text) noexcept
			: text_(expection_text)
		{}
		~my_exception() {}
	private:
		std::string text_;
	};

	class ImageData
	{
	private:
		int width_ = 0;
		int height_= 0;
		int numComponets_ = 0;
		IO::DataArray::Ptr pixelsArray_;
		int scanline_count_ = 0;
	public:
		ImageData(int width, int height, int output_components)
			: width_(width)
			, height_(height)
			, numComponets_(output_components)
		{
			int pixels_size = width * height * output_components;
			pixelsArray_ = std::move(IO::makeDataArray(pixels_size));
		}
		ImageData()	{}

		IO::ByteArray getData()
		{
			return pixelsArray_->data();
		}
		IO::ByteArray getData() const
		{
			return pixelsArray_->data();
		}
		uint32_t getSize() const
		{
			return pixelsArray_->size();
		}
		void setScanlineCount(const int scanline_count)
		{
			scanline_count_ = scanline_count;
		}
		int getScanlineCount() const
		{
			return scanline_count_;
		}
		int getWidth() const
		{
			return width_;
		}
		int getHeight() const
		{
			return height_;
		}
		int getOutputComponents() const
		{
			return numComponets_;
		}

	};

	double calcPercentages(int val, int max)
	{
		if (max == 0)
			return 0.0;
		double percent = (double)val / (double)max;
		return percent * 100.00;
	}

	void jpegErrorExit_throw(j_common_ptr cinfo)
	{
		char jpegLastErrorMsg[JMSG_LENGTH_MAX];
		/* Create the message */
		(*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);

		/* Jump to the setjmp point */
		//throw std::runtime_error(jpegLastErrorMsg); // or your preffered exception ...
		throw my_exception(jpegLastErrorMsg);
	}

	void jpegErrorExit_nothrow(j_common_ptr cinfo)
	{
		char jpegLastErrorMsg[JMSG_LENGTH_MAX];
		/* Create the message */
		(*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);

		/* Jump to the setjmp point */
		//throw std::runtime_error(jpegLastErrorMsg); // or your preffered exception ...
		//throw my_exception(jpegLastErrorMsg);
	}

	bool isVaidMSGCode(const int msg_code)
	{
		if (msg_code >= 73/*JMSG_COPYRIGHT*/
			&& msg_code <= 113/*JTRC_XMS_OPEN*/)
			return true;
		return false;
	}
	void jpegOutputMessage(j_common_ptr cinfo)
	{
		char jpegLastErrorMsg[JMSG_LENGTH_MAX];
		/* Create the message */
		(*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);
		printf("%s (error_code [%d])\n", jpegLastErrorMsg, cinfo->err->msg_code);
		//printf("Error code [%d]\n", cinfo->err->msg_code);
		//if (!isVaidMSGCode(cinfo->err->msg_code))
		//	printf("Not Valid error code");


		/* Jump to the setjmp point */
		//if (!isVaidMSGCode(cinfo->err->msg_code))
		//	throw std::runtime_error(jpegLastErrorMsg); // or your preffered exception ...
	}

	void saveBMP_file(const IO::path_string & path , const ImageData & image_data)
	{
		IO::File bmp_file(path);
		bmp_file.Open(IO::OpenMode::Create);


		ImageData img_new(image_data.getWidth(), image_data.getHeight(), image_data.getOutputComponents());
		auto src_data = (ByteArray) image_data.getData();
		auto byteArray = img_new.getData();
		for (auto i = 0; i < image_data.getSize(); i = i + image_data.getOutputComponents())
		{
			for (auto j = 0; j < image_data.getOutputComponents(); ++j)
				byteArray[i + j] = src_data[image_data.getSize() - 1 - i - j];
		}


		BITMAPINFOHEADER bfh = BITMAPINFOHEADER();
		//BITMAPINFO bi;
		bfh.biSize = sizeof(BITMAPINFOHEADER);
		bfh.biHeight = image_data.getHeight();
		bfh.biWidth = image_data.getWidth();
		bfh.biBitCount = 24;
		bfh.biPlanes = 1;
		bfh.biCompression = BI_RGB;

		const char BM_TEXT[] = "BM";

		BITMAPFILEHEADER bm_header = BITMAPFILEHEADER();
		memcpy(&bm_header.bfType, BM_TEXT, 2);
		bm_header.bfSize = sizeof(BITMAPFILEHEADER) + bfh.biSize + image_data.getSize();


		auto bytes_written = bmp_file.WriteData((ByteArray)&bm_header, sizeof(BITMAPFILEHEADER));
		bytes_written += bmp_file.WriteData((ByteArray)&bfh, bfh.biSize);
		bytes_written += bmp_file.WriteData(img_new.getData(), img_new.getSize());
		//bytes_written += bmp_file.WriteData(byteArray, image_data.getSize());

		bmp_file.Close();


	}

	class Jpegdata
	{
	private:
		//std::unique_ptr<jpeg_decompress_struct> jpg_struct_ptr_;
		jpeg_error_mgr err = jpeg_error_mgr();
		jpeg_decompress_struct cinfo = jpeg_decompress_struct();
	public:
		Jpegdata()
		{
			createDecompressor();
		}
		~Jpegdata()
		{
			destroyDecompressor();
		}
		void Init()
		{
			ZeroMemory(&err, sizeof(jpeg_error_mgr));
			ZeroMemory(&cinfo, sizeof(jpeg_decompress_struct));
		}
		void createDecompressor()
		{
			Init();
			jpeg_create_decompress(&cinfo);
			cinfo.err = jpeg_std_error(&err);
			cinfo.do_fancy_upsampling = FALSE;
			err.error_exit = jpegErrorExit_throw;
			err.output_message = jpegOutputMessage;
			//err.trace_level = 1;
		}
		jpeg_decompress_struct * getDecompressStruct()
		{
			return &cinfo;
		}
		void destroyDecompressor()
		{
			err.error_exit = jpegErrorExit_nothrow;
			jpeg_destroy_decompress(&cinfo);
		}

	};


	class JpegDecoder
	{
	private:
		//using jpg_struct_ptr = std::unique_ptr<jpeg_decompress_struct> ;
		//jpeg_error_mgr err = jpeg_error_mgr();
		//jpeg_decompress_struct cinfo = jpeg_decompress_struct();
		std::unique_ptr<Jpegdata> jpgPtr_ = std::make_unique<Jpegdata>();
	public:
		//JpegDecoder(const IO::path_string & jpeg_filename)
		//	: jpeg_filename_(jpeg_filename)
		//{

		//}

		IO::DataArray ReadJpegFile(const IO::path_string & jpeg_filename)
		{
			IO::File jpg_file(jpeg_filename);
			if (!jpg_file.Open(IO::OpenMode::OpenRead))
				return IO::DataArray(0);

			IO::DataArray data_array(jpg_file.Size());
			if (data_array.size() == 0)
				return data_array;

			auto bytes_read = jpg_file.ReadData(data_array);
			if (bytes_read != data_array.size())
				return IO::DataArray(0);

			return data_array;
		}

		ImageData decompress(const IO::path_string & jpeg_filename)
		{
			//createDecompressor();

			auto data_array = ReadJpegFile(jpeg_filename);
			if (data_array.size() == 0)
				return ImageData();

			auto cinfo_ptr = jpgPtr_->getDecompressStruct();
			
			auto err_ptr = cinfo_ptr->err;
			err_ptr->trace_level = 3;

			/* set source buffer */
			jpeg_mem_src(cinfo_ptr, data_array.data(), data_array.size());

			/* read jpeg header */
			jpeg_read_header(cinfo_ptr, 1);


			auto bRes = jpeg_start_decompress(jpgPtr_->getDecompressStruct());
			if (err_ptr->num_warnings > 1)
			{
				return ImageData();
			}
			//err_ptr->num_warnings = 0;

			JSAMPROW output_data;
			auto scanline_len = cinfo_ptr->output_width * cinfo_ptr->output_components;

			auto scanline_count = 0;
			ImageData image_data(cinfo_ptr->output_width, cinfo_ptr->output_height, cinfo_ptr->output_components);


			//err_ptr->msg_code = 85;

			while (cinfo_ptr->output_scanline < cinfo_ptr->output_height)
			{
				output_data = (image_data.getData() + (scanline_count * scanline_len));
				jpeg_read_scanlines(cinfo_ptr, &output_data, 1);

				if (err_ptr->num_warnings > 1)
					break;

				if (!isVaidMSGCode(err_ptr->msg_code))
				{
					//int k = cinfo.next_scanline;
					//cinfo.output_scanline = cinfo.output_height;
					int k = 0;
					k = 1;
					break;
				}

				scanline_count++;
			}

			cinfo_ptr->output_scanline = cinfo_ptr->output_height;	// to prevent exception 			
			jpeg_finish_decompress(cinfo_ptr);
			image_data.setScanlineCount(scanline_count);

//			saveBMP_file(L"d:\\Photo\\jpg_test\\not_bad\\bitmap.bmp" , image_data);

 			return image_data;
		}

	};

	class JpegTester
	{
	public:
		void test_jpeg_files(const path_string & src_folder , const double border_percenteges)
		{
			IO::Finder finder;
			finder.add_extension(L".jpg");
			finder.add_extension(L".jpeg");
			finder.FindFiles(src_folder);
			auto fileList = finder.getFiles();
			for (auto & theFile : fileList)
			{
				fs::path src_path(theFile);
				auto folder_path = src_path.parent_path().generic_wstring();
				auto only_name_path = src_path.stem().generic_wstring();
				auto ext = src_path.extension().generic_wstring();

				IO::path_string new_file_name = theFile + L".bad_file";//bad_folder + only_name_path + ext;


				try
				{
					IO::JpegDecoder jpeg_decoder;
					auto img_data = jpeg_decoder.decompress(theFile);
					auto percenteges = IO::calcPercentages(img_data.getScanlineCount(), img_data.getHeight());

					auto perc = std::lround(percenteges);
					auto name_percenteges = std::to_wstring(perc);
					std::replace(name_percenteges.begin(), name_percenteges.end(), '.', '-');

					if (percenteges > border_percenteges)
					{
						wprintf(L"%s - GOOD\n", theFile.c_str());

						new_file_name = theFile;
						//new_file_name = dst_folder + only_name_path + L" [" + name_percenteges + L"]" + ext;
						continue;

					}
					//else
					//	new_file_name = bad_folder + only_name_path + L" [" + name_percenteges + L"]" + ext;
				}
				catch (IO::my_exception & ex)
				{
					printf("%s\n", ex.what());
				}
				catch (...)
				{
					printf("FATAL ERROR\n");
				}

				wprintf(L"%s - FAILED\n", theFile.c_str());

				try
				{
					fs::rename(theFile, new_file_name);
				}
				catch (const fs::filesystem_error& e)
				{
					std::cout << "Error: " << e.what() << std::endl;
				}

			}

		}

	};
}