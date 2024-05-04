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

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() : disk(nullptr), size(MAX_INODES), inodes(nullptr), free_blocks(nullptr) {
    Console::puts("In file system constructor.\n");

    inodes = new Inode[MAX_INODES];
    if (!inodes) {
        Console::puts("Failed to allocate inodes.\n");
        assert(false);
    }

    for (int i = 0; i < MAX_INODES; i++) {
        inodes[i].id = -1;
        inodes[i].fs = this;
    }

    free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];
    if (!free_blocks) {
        Console::puts("Failed to allocate free blocks.\n");
        assert(false);
    }

    memset(free_blocks, 0, SimpleDisk::BLOCK_SIZE);
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    // assert(false);

    if (disk) {
        unsigned char inode_data[SimpleDisk::BLOCK_SIZE];
        unsigned char free_data[SimpleDisk::BLOCK_SIZE];

        memcpy(inode_data, inodes, sizeof(Inode) * MAX_INODES);

        disk->write(0, inode_data);

        memcpy(free_data, free_blocks, SimpleDisk::BLOCK_SIZE);

        disk->write(1, free_data);
    }

    if (inodes) {
        delete[] inodes;
        inodes = nullptr;
    }

    if (free_blocks) {
        delete[] free_blocks;
        free_blocks = nullptr;
    }
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk* _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    // assert(false);

    if (!_disk) {
        return false;
    }

    this->disk = _disk;

    unsigned char inode_data[SimpleDisk::BLOCK_SIZE];
    unsigned char free_data[SimpleDisk::BLOCK_SIZE];

    disk->read(0, inode_data);

    memcpy(inodes, inode_data, sizeof(Inode) * MAX_INODES);

    disk->read(1, free_data);

    memcpy(free_blocks, free_data, SimpleDisk::BLOCK_SIZE);

    return true;
}

bool FileSystem::Format(SimpleDisk* _disk, unsigned int _size) {  // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    // assert(false);
    if (!_disk) {
        return false;
    }

    Inode local_inodes[MAX_INODES];
    unsigned char local_free_blocks[SimpleDisk::BLOCK_SIZE];

    for (int i = 0; i < MAX_INODES; i++) {
        local_inodes[i].id = -1;
    }

    memset(local_free_blocks, 0, SimpleDisk::BLOCK_SIZE);

    int inode_block_num = 0;
    int free_block_num = 1;

    local_free_blocks[inode_block_num / 8] |= 1 << (inode_block_num % 8);
    local_free_blocks[free_block_num / 8] |= 1 << (free_block_num % 8);

    _disk->write(inode_block_num, (unsigned char*)local_inodes);

    _disk->write(free_block_num, local_free_blocks);

    return true;
}

Inode* FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = ");
    Console::puti(_file_id);
    Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    // assert(false);
    for (unsigned int i = 0; i < this->size; i++) {
        if (this->inodes[i].id == _file_id) {
            return &this->inodes[i];
        }
    }

    return nullptr;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:");
    Console::puti(_file_id);
    Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    // assert(false);

    if (this->LookupFile(_file_id) != nullptr) {
        Console::puts("File already exists.\n");
        return false;
    }

    // get free inode
    Inode* inode = GetFreeInode();
    if (!inode) {
        Console::puts("Failed to get free inode.\n");
        return false;
    }

    inode->id = _file_id;
    inode->fs = this;

    int free_block = GetFreeBlock();
    if (free_block == -1) {
        Console::puts("Failed to get free block.\n");
        return false;
    }

    inode->block_no = free_block;

    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:");
    Console::puti(_file_id);
    Console::puts("\n");
    /* First, check if the file exists. If not, throw an error.
       Then free all blocks that belong to the file and delete/invalidate
       (depending on your implementation of the inode list) the inode. */

    Inode* inode = this->LookupFile(_file_id);
    if (!inode) {
        return false;
    }

    int block_to_free = inode->block_no;
    if (block_to_free != -1) {
        this->free_blocks[block_to_free] = 0;  // mark as free
    }

    inode->id = -1;
    inode->block_no = -1;

    return true;
}

Inode* FileSystem::GetFreeInode() {
    for (unsigned int i = 0; i < this->size; i++) {
        if (this->inodes[i].id == -1) {
            return &this->inodes[i];
        }
    }

    return nullptr;
}

int FileSystem::GetFreeBlock() {
    for (unsigned int i = 0; i < SimpleDisk::BLOCK_SIZE; i++) {
        if (this->free_blocks[i] == -1) {
            this->free_blocks[i] = 1;  // mark as used
            return i;
        }
    }

    return -1;
}
