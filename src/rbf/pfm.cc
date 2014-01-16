#include "pfm.h"
#include<iostream>
using namespace std;

// Check if a file exists
bool FileExists(string fileName)
{
	struct stat stFileInfo;

	if(stat(fileName.c_str(), &stFileInfo) == 0) return true;
	else return false;
}

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
	if(!_pf_manager)
		_pf_manager = new PagedFileManager();

	return _pf_manager;
}

// The PagedFileManager does nothing.
PagedFileManager::PagedFileManager()
{

}

// The PagedFileDestructor does nothing
PagedFileManager::~PagedFileManager()
{

}

// <createFile> tells the OS to CREATE a file.
// if the fileName exists the function returns an error code '1' else it CREATES the file.
// The function also maintains a set to keep record of all files created by the function
// ARGS:
// fileName: const char* (c - string)
RC PagedFileManager::createFile(const char *fileName)
{
	if(FileExists(fileName))
	return -1;

	FILE *file;
	file = fopen(fileName,"wb");
	fclose(file);

	files.insert(fileName);
	return 0;
}

// <destroyFile> tells the OS to DESTROY a file.
// if the fileName does not exist the function returns an error code '1' else it DESTROYS the file.
// ARGS:
// fileName: const char* (c - string)
RC PagedFileManager::destroyFile(const char *fileName)
{
	if(!FileExists(fileName))
    return -1;

	remove(fileName);
	return 0;
}


RC PagedFileManager::openFile(const char *fileName, FileHandle &fileHandle)
{

//	//If entry exists , then open in read mode.
//	If entry does not exist then returnerror.
//   we  have steram attributein handle.



   return -1;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
//	Ii need to dealloc stream
    return -1;
}


FileHandle::FileHandle()
{
	stream = 0;
}


FileHandle::~FileHandle()
{

}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
//	This method reads the page into the memory block pointed by data.
//  The page should exist. Note the page number starts from 0.
//	See if pageunum eceeds the numofpages. If so errro.
//	Read pagesize data using fread.

    return -1;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{

//	This method writes the data into a page specified by the pageNum. The page should exist. Note the page number starts from 0.
//	Similar to fread.
//	Refer example for writing.

    return -1;
}


RC FileHandle::appendPage(const void *data)
{
	return -1;
}


unsigned FileHandle::getNumberOfPages()
{
	return -1;
}


