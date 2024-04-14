/*
 File: scheduler.C

 Author:
 Date  :

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
#include "scheduler.H"
#include "simple_keyboard.H"
#include "thread.H"
#include "utils.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

queue::queue() {
    head = nullptr;
    tail = nullptr;
}

void queue::add(Thread* _thread) {
    ThreadNode* newNode = new ThreadNode(_thread);

    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

Thread* queue::pop() {
    if (head == nullptr) {
        return nullptr;
    }

    Thread* _thread = head->thread;
    ThreadNode* temp = head;

    head = head->next;
    if (head == nullptr) {
        tail = nullptr;
    }

    delete temp;
    return _thread;
}

void queue::remove(Thread* _thread) {
    ThreadNode* current = head;
    ThreadNode* previous = nullptr;

    while (current != nullptr) {
        if (current->thread == _thread) {
            if (previous == nullptr) {
                head = current->next;
            } else {
                previous->next = current->next;
            }

            if (current == tail) {
                tail = previous;
            }

            delete current;
            return;
        }

        previous = current;
        current = current->next;
    }
}

bool queue::isEmpty() {
    return head == nullptr;
}

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
    currentThread = nullptr;
}

void Scheduler::yield() {
    if (!readyQueue.isEmpty()) {
        currentThread = readyQueue.pop();
        Thread::dispatch_to(currentThread);
    }
}

void Scheduler::resume(Thread* _thread) {
    readyQueue.add(_thread);
}

void Scheduler::add(Thread* _thread) {
    readyQueue.add(_thread);
}

void Scheduler::terminate(Thread* _thread) {
    readyQueue.remove(_thread);
}
