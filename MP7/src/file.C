/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
	inode = _fs->LookupFile(_id);
	current_position = 0;
	_fs->ReadBlock(inode->block, block_cache);	
	fs = _fs;
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
	fs->WriteBlock(inode->block, block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
	// make sure read doesn't exceed file size
	if (current_position + _n > inode->size) {
		_n = inode->size - current_position;
	}
	// copy data from cache to buffer
	for (int i = 0; i < _n; i++) {
		_buf[i] = block_cache[current_position + i];
	}
	// update current position
	current_position += _n;
	return _n;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
	// make sure write doesn't exceed max file size
	if (current_position + _n > MAX_FILE_SIZE) {
		Console::puts("write exceeds max file size\n");
		return -1;
	}
	// place data in cache
	for (int i = 0; i < _n; i++) {
		block_cache[current_position + i] = _buf[i];
	}
	// update current position
	current_position += _n;
	// update inode size to max of current position and current size
	inode->size = current_position > inode->size ? current_position : inode->size;
	return _n;
}

void File::Reset() {
    Console::puts("resetting file\n");
	current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
	return current_position == inode->size;
}
