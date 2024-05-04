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

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) : fs(_fs), file_id(_id), current_position(0) {
    Console::puts("Opening file.\n");
    // assert(false);

    inode = fs->LookupFile(_id);
    if (!inode) {
        Console::puts("Failed to find inode.\n");
        assert(false);
    }

    disk_block = new unsigned char[SimpleDisk::BLOCK_SIZE];
    fs->disk->read(inode->block_no, block_cache);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */

    fs->disk->write(inode->block_no, block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    // assert(false);

    int bytes_read = 0;
    while (_n > 0 && current_position < SimpleDisk::BLOCK_SIZE) {
        _buf[bytes_read] = block_cache[current_position];
        bytes_read++;
        current_position++;
        _n--;
    }

    return bytes_read;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    // assert(false);

    int bytes_written = 0;
    while (_n > 0 && current_position < SimpleDisk::BLOCK_SIZE) {
        block_cache[current_position] = _buf[bytes_written];
        bytes_written++;
        current_position++;
        _n--;
    }

    return bytes_written;
}

void File::Reset() {
    Console::puts("resetting file\n");
    // assert(false);
    current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    // assert(false);

    return current_position >= SimpleDisk::BLOCK_SIZE;
}
