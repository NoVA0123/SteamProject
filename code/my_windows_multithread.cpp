/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <cassert>
#include <cstdint>
#include <windows.h>
#include <winbase.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdint.h>


#define CompletePastWritesBeforeFutureWrites
#define CompletePastReadsBeforeFutureReads 
typedef uint32_t u32;


struct work_queue_entry_storage {
    void * UserPointer;
};

// Volatile strictly tells compiler not to optimze this code and it creates a _WriteBarrier for the variable
struct work_queue {
    u32 volatile EntryCompletionCount;
    u32 volatile NextEntryToDo;
    u32 volatile EntryCount;
    HANDLE SemaphoreHandle;
    work_queue_entry_storage Entries[256];
};

static void AddWorkQueue(struct work_queue * Queue, void * Pointer) {
    // _WriteBarrier tells compiler not to reorder the stores
    // _mm_sfence tells processor not to reorder the stores
    assert(Queue->EntryCount < 256);
    Queue->Entries[Queue->EntryCount].UserPointer = Pointer;
    _WriteBarrier();
    _mm_sfence();
    ++(Queue -> EntryCount);
    ReleaseSemaphore(Queue -> SemaphoreHandle, 1, 0);
}

struct work_queue_entry {
    bool IsValid;
    void *Data;
};

static work_queue_entry CompleteAndGetNextWorkQueueItem(
        work_queue * Queue,
        work_queue_entry Completed) {
    work_queue_entry Result;
    Result.IsValid = false;

    if (Completed.IsValid) {
        InterlockedIncrement((LONG volatile*) &Queue -> EntryCompletionCount);
    }
    u32 OriginalNextEntryToDo = Queue->NextEntryToDo;
    if (OriginalNextEntryToDo < Queue -> EntryCount) {
        u32 Index = InterlockedCompareExchange(
                (LONG volatile *) &Queue -> NextEntryToDo,
                OriginalNextEntryToDo + 1,
                OriginalNextEntryToDo);
        if (Index == OriginalNextEntryToDo) {
            Result.Data = Queue->Entries[Index].UserPointer;
            Result.IsValid = true;
            _ReadBarrier();
        }
    }
    return Result;
}

static bool QueueWorkStillInProgress(work_queue * Queue) {
    bool Result = (Queue -> EntryCount != Queue->EntryCompletionCount);
    return Result;
}


inline void ThreadDoWork(work_queue_entry Entry, int LogicalThreadIndex) {
    assert(Entry.IsValid);
    char buffer[256];
    sprintf(buffer, "Thread %u: %s\n", LogicalThreadIndex, (char*)Entry.Data);
    OutputDebugString(buffer);
}

struct win32_thread_info {
    u32 LogicalThreadIndex;
    work_queue * Queue;
};

DWORD WINAPI
ThreadProc(LPVOID lpParameter) {
    win32_thread_info *ThreadInfo = (win32_thread_info*) lpParameter;
    work_queue_entry Entry = {};
    for (;;) {
        Entry = CompleteAndGetNextWorkQueueItem(ThreadInfo->Queue, Entry);
        if (Entry.IsValid) {
            ThreadDoWork(Entry, ThreadInfo->LogicalThreadIndex);
        } else {
            WaitForSingleObjectEx(ThreadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
//    return (0);
}

static void PushString(work_queue * Queue, char * String) {
    AddWorkQueue(Queue, String);
}

int main() {
    win32_thread_info ThreadInfo[4];
    u32 ThreadCount = 4;
    u32 InitialCount = 0;
    work_queue Queue = {};
    Queue.SemaphoreHandle = CreateSemaphoreEx(
            0,
            InitialCount,
            ThreadCount,
            0,
            0,
            SEMAPHORE_ALL_ACCESS);


    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex) {
        win32_thread_info * Info = ThreadInfo + ThreadIndex;
        Info -> Queue = &Queue;
        Info -> LogicalThreadIndex = ThreadIndex;
        DWORD ThreadId;
        /*
         Params are:
         1. Allowing thread to handle windows api calls.
         2. Setting stack size
         3. Function name
         4. Parameter passtrhrough
         5. Whether to run thread Immediately
         6. Thread Id
         */
        HANDLE ThreadHandle = CreateThread(
                0,
                0,
                ThreadProc,
                Info,
                0,
                &ThreadId);
        CloseHandle(ThreadHandle);
    }
    PushString(&Queue,"String: A0");
    PushString(&Queue,"String: A1");
    PushString(&Queue,"String: A2");
    PushString(&Queue,"String: A3");
    PushString(&Queue,"String: A4");
    PushString(&Queue,"String: A5");
    PushString(&Queue,"String: A6");
    PushString(&Queue,"String: A7");
    PushString(&Queue,"String: A8");
    PushString(&Queue,"String: A9");

    Sleep(5000);

    PushString(&Queue,"String: B0");
    PushString(&Queue,"String: B1");
    PushString(&Queue,"String: B2");
    PushString(&Queue,"String: B3");
    PushString(&Queue,"String: B4");
    PushString(&Queue,"String: B5");
    PushString(&Queue,"String: B6");
    PushString(&Queue,"String: B7");
    PushString(&Queue,"String: B8");
    PushString(&Queue,"String: B9");

    work_queue_entry Entry = {};
    while (QueueWorkStillInProgress(&Queue)) {
        Entry = CompleteAndGetNextWorkQueueItem(&Queue, Entry);
        if (Entry.IsValid) {
            ThreadDoWork(Entry, 4);
        }
    }
    return 0;
}
