#pragma once

#include "resource.h"



//#include "IOLibrary/iofunctions.h"
#include "IO/constants.h"
#include <iostream>


void identify_files(const std::string & source_dir, const std::string & target_dir);
bool identify_pdf(const IO::path_string & file_name, IO::path_string & new_filename, int counter);
bool open_pdf_file(const IO::path_string & file_name);
void verify_pdf_files(const IO::path_string & folder);

