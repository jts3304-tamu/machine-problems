/*
     File        : blocking_disk.c

     Author      :
     Modified    :

     Description :

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "blocking_disk.H"
#include "console.H"
#include "scheduler.H"

#ifdef _USES_SCHEDULER_

extern Scheduler* SYSTEM_SCHEDULER;

#endif

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
    : SimpleDisk(_disk_id, _size) {
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char* _buf) {
    SimpleDisk::issue_operation(DISK_OPERATION::READ, _block_no);

#ifdef _USES_SCHEDULER_
    // yield until disk is ready (polling)
    // this is not busy waiting, but rather a way to yield the CPU
    // without having to implement a blocked queue
    while (!SimpleDisk::is_ready()) SYSTEM_SCHEDULER->yield();
#endif

#ifndef _USES_SCHEDULER_
    SimpleDisk::wait_until_ready();
#endif

    // read data from port
    for (int i = 0; i < 256; i++) {
        unsigned short data = Machine::inportw(0x1F0);
        _buf[i * 2] = (unsigned char)data;
        _buf[i * 2 + 1] = (unsigned char)(data >> 8);
    }
}

void BlockingDisk::write(unsigned long _block_no, unsigned char* _buf) {
    SimpleDisk::issue_operation(DISK_OPERATION::WRITE, _block_no);

#ifdef _USES_SCHEDULER_
    // yield until disk is ready (polling)
    while (!SimpleDisk::is_ready()) SYSTEM_SCHEDULER->yield();
#endif

#ifndef _USES_SCHEDULER_
    SimpleDisk::wait_until_ready();
#endif

    // write data to port
    for (int i = 0; i < 256; i++) {
        unsigned short data = _buf[i * 2] | (_buf[i * 2 + 1] << 8);
        Machine::outportw(0x1F0, data);
    }
}
