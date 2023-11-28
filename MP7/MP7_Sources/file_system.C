/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"

// grab memory management from kernel.C
typedef long unsigned int size_t;
extern void * operator new (size_t size);
extern void * operator new[] (size_t size);
extern void operator delete (void * p, size_t s);
extern void operator delete[] (void * p);
/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

Inode::Inode() : id(-1), size(-1), block(-1), fs(NULL) {}

Inode::Inode(long _id, long _size, long _block, FileSystem * _fs) :
	id(_id), size(_size), block(_block), fs(_fs) {}

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
	// initialize the disk to NULL
	disk = NULL;
	// allocate memory for the inode list (1 block)
	inodes = (Inode*)new unsigned char[SimpleDisk::BLOCK_SIZE];
	// initialize the free list (1 block)
	free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
	// if we mounted a disk, write the inode list back to disk
	if (disk != NULL) {
		disk->write(0, (unsigned char *)inodes);
		// also flush the free list
		disk->write(1, free_blocks);
	}
    assert(false);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");
	assert(disk == NULL);
	assert(_disk != NULL);
	// set the disk pointer
	disk = _disk;
	// read the inode list from disk
	disk->read(0, (unsigned char *)inodes);
	// read the free list from disk
	disk->read(1, free_blocks);
	return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
	Inode *blank_inodes = (Inode*)new unsigned char[SimpleDisk::BLOCK_SIZE];
	// set all inodes to unused
	for (int i = 0; i < MAX_INODES; i++) {
		blank_inodes[i] = Inode();
	}
	unsigned char *blank_free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];
	// set inode block to used
	blank_free_blocks[0] = 1;
	// set free list block to used
	blank_free_blocks[1] = 1;
	Console::puts("using 2 blocks for inode list and free list\n");
	// set all other blocks to free
	for (int i = 2; i < _size/SimpleDisk::BLOCK_SIZE; i++) {
		blank_free_blocks[i] = 0;
	}
	Console::puts("setting ");
	Console::puti(_size/SimpleDisk::BLOCK_SIZE - 2);
	Console::puts(" blocks to free\n");
	// and the rest of the blocks to unavailable
	for (int i = _size/SimpleDisk::BLOCK_SIZE; i < SimpleDisk::BLOCK_SIZE; i++) {
		blank_free_blocks[i] = 2;
	}
	Console::puts("setting ");
	Console::puti(SimpleDisk::BLOCK_SIZE - _size/SimpleDisk::BLOCK_SIZE);
	Console::puts(" blocks to unavailable\n");
	// write the inode list to disk
	_disk->write(0, (unsigned char *)blank_inodes);
	// write the free list to disk
	_disk->write(1, blank_free_blocks);
	// delete the temporary inode list
	delete[] blank_inodes;
	// delete the temporary free list
	delete[] blank_free_blocks;
	return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    assert(false);
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
	// loop over all inodes and make sure the file doesn't already exist
	for (int i = 0; i < MAX_INODES; i++) {
		if (inodes[i].id == _file_id) {
			Console::puts("file already exists\n");
			return false;
		}
	}
	// get a free inode
	Inode *inode = GetFreeInode();
	// if there are no free inodes, return false
	if (inode == NULL) {
		Console::puts("no free inodes\n");
		return false;
	}
	// set the inode info
	inode->id = _file_id;
	inode->size = 0;
	inode->block = -1;
	inode->fs = this;

	return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
}

Inode * FileSystem::GetFreeInode() {
	Console::puts("getting free inode\n");
	// loop over all inodes
	for (int i = 0; i < MAX_INODES; i++) {
		// if the inode is unused, return it
		if (inodes[i].id == -1) {
			Console::puts("found free inode\n");
			return &inodes[i];
		}
	}
	// if there are no free inodes, return NULL
	Console::puts("no free inodes\n");
	return NULL;
}
