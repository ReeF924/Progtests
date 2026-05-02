#ifndef __PROGTEST__
#include "common.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <semaphore.h>
using namespace std;
#endif /* __PROGTEST__ */

void initializePageTable(uint32_t *tablePtr) {
  // set the present bit to 0 everywhere
  for (int i = 0; i < 1024; i++) {
    tablePtr[i] = 0;
  }
}

class uniqueLock {
  pthread_mutex_t &mutex;
  bool unlocked = false;

public:
  uniqueLock(pthread_mutex_t &mutex) : mutex(mutex) { pthread_mutex_lock(&mutex); }

  ~uniqueLock() {
    if (unlocked)
      return;

    unlock();
  }

  void unlock() {
    pthread_mutex_unlock(&mutex);
    unlocked = true;
  }
};

class Process;

struct MemManager {
  Process *processes[64];
  uint8_t *mem;

  bool *pagesBitmap;
  uint32_t bitMapPtr = 0;
  uint32_t totalPages;

  static constexpr unsigned TABLE_ROW_COUNT = 1024;

  int activeProcCount = 0;
  pthread_mutex_t processMutex;
  pthread_cond_t processCv;
  pthread_mutex_t pagesMutex;

  MemManager(void *mem, uint32_t totalPages)
      : mem((uint8_t *)mem), totalPages(totalPages) /*totalPages((totalPages + 32 - 1) / 32)*/ {
    // pagesBitmap = new uint32_t[totalPages]();
    pagesBitmap = new bool[totalPages]();

    pthread_mutex_init(&processMutex, nullptr);
    pthread_mutex_init(&pagesMutex, nullptr);
    pthread_cond_init(&processCv, nullptr);
  }

  ~MemManager() = default;

  void allocPage(uint32_t pageIndex);
  void freePage(uint32_t pageIndex);
  void run(void *processArg, void (*mainProcess)(CCPU *, void *));
  uint32_t *findFreePages(uint32_t pages);
  uint32_t findFreePage();
};

struct ThreadArg {
  Process *proc;
  MemManager &memMng;
  void (*entryPoint)(CCPU *, void *);
  void *arg;

  ThreadArg(Process *proc, MemManager &mem, void (*entryPoint)(CCPU *, void *), void *arg)
      : proc(proc), memMng(mem), entryPoint(entryPoint), arg(arg) {}
};

void *threadEntry(void *arg);

class Process : public CCPU {
  MemManager &memMng;
  uint32_t pagesCount = 0;
  uint32_t threadId;

public:
  Process(uint8_t *memStart, uint32_t pageTableRoot, MemManager &mem) : CCPU(memStart, pageTableRoot), memMng(mem) {}

  ~Process() override {
    SetMemLimit(0);

    uniqueLock lock(memMng.pagesMutex);
    memMng.freePage(m_PageTableRoot >> OFFSET_BITS);
  }

  [[nodiscard]] uint32_t GetMemLimit() const override { return pagesCount; }

  bool SetMemLimit(uint32_t pages) override {
    //printf("SetMemLimit: old=%u new=%u\n", pagesCount, pages);
    if (pages == pagesCount) {
      return true;
    }

    int pagesChange = static_cast<int>(pages - pagesCount);

    // lower the amount of pages
    if (pagesChange < 0) {
      reducePagesCount(pages);
      //printf("DONE: pagesCount = %u\n", pagesCount);
      return true;
    }

    uniqueLock lock(memMng.pagesMutex);

    uint32_t pagesNeeded = getNeededPages(static_cast<uint32_t>(pagesChange));

    auto *freeArr = memMng.findFreePages(static_cast<uint32_t>(pagesNeeded));
    auto *pagePtr = freeArr;

    // not enough pages left
    if (freeArr == nullptr) {
      return false;
    }

    uint32_t index = 0;
    for (uint32_t p = pagesCount; p < pages; ++p) {
      mapPage(p, pagePtr, index);
    }

    lock.unlock();

    delete[] freeArr;

    pagesCount = pages;
    //printf("DONE: pagesCount = %u\n", pagesCount);
    return true;
  }

  bool NewProcess(void *processArg, void (*entryPoint)(CCPU *, void *), bool copyMem) override {
    //printf("NewProcess: \n");
    Process *childProc = new Process(m_MemStart, 0, memMng);

    uniqueLock procLock(memMng.processMutex);

    while (memMng.activeProcCount >= 64) {
      pthread_cond_wait(&memMng.processCv, &memMng.processMutex);
    }
    ++memMng.activeProcCount;
    uint32_t procIdx = findFreeProcSlot();
    memMng.processes[procIdx] = childProc;

    uniqueLock pagesLock(memMng.pagesMutex);

    if (copyMem) {
      //printf("CopyMem: \n");

      uint32_t dataPages = pagesCount;
      uint32_t subTables = (pagesCount + 1023) / 1024;
      uint32_t totalNeeded = 1 + subTables + dataPages;

      auto freePagesArr = memMng.findFreePages(totalNeeded);
      uint32_t index = 0;

      if (!freePagesArr) {
        memMng.processes[procIdx] = nullptr;
        pagesLock.unlock();
        delete childProc;
        return false;
      }

      uint32_t mainPageIdx = freePagesArr[index++];
      memMng.allocPage(mainPageIdx);
      childProc->m_PageTableRoot = mainPageIdx << OFFSET_BITS;
      initializePageTable(childProc->mainPageTable());

      // already alloced main page
      --totalNeeded;
      // now copy
      for (uint32_t i = 0; i < pagesCount; ++i) {
        auto *dest = childProc->mapPage(i, freePagesArr, index);
        auto *src = virtual2Physical(i << OFFSET_BITS, false);

        memcpy(dest, src, (1 << OFFSET_BITS));
      }
      childProc->pagesCount = pagesCount;

    } else {
      //printf("NotCopyMem: \n");
      uint32_t mainPageIdx = memMng.findFreePage();

      // TODO: Maybe change the zero...(in the findFreePage func)
      if (mainPageIdx == 0) {
        memMng.processes[procIdx] = nullptr;
        pagesLock.unlock();
        delete childProc;
        return false;
      }

      memMng.allocPage(mainPageIdx);
      childProc->m_PageTableRoot = mainPageIdx << OFFSET_BITS;
      initializePageTable(childProc->mainPageTable());
      childProc->pagesCount = 0;
    }

    //printf("StartingThread: \n");
    ThreadArg *threadArg = new ThreadArg(childProc, memMng, entryPoint, processArg);
    pthread_t tid;
    pthread_create(&tid, nullptr, threadEntry, threadArg);
    pthread_detach(tid);

    //printf("DoneNewProcess: \n");
    return true;
  }

protected:
  /*
   if copy-on-write is implemented:

   virtual bool             pageFaultHandler              ( uint32_t address,
                                                            bool write );
   */

private:
  void reducePagesCount(uint32_t pagesChanged) {
    uniqueLock lock(memMng.pagesMutex);
    //printf("Reducing from %u to %u\n", pagesCount, pagesChanged);
    // get address of the main table
    auto mainPageTable = (uint32_t *)(m_MemStart + (m_PageTableRoot & ADDR_MASK));

    for (uint32_t virtualPage = pagesChanged; virtualPage < pagesCount; ++virtualPage) {

      // row where the virtual page is referenced
      uint32_t mainPageRow = mainPageTable[virtualPage >> 10];

      auto *subPage = (uint32_t *)(m_MemStart + (mainPageRow & ADDR_MASK));

      // index of the virtualPage in SubTable
      uint32_t subRowIndex = virtualPage & 0x3FF;
      uint32_t &subPageRow = subPage[subRowIndex];

      // convert it into index and free it
      uint32_t pageIdx = (subPageRow & ADDR_MASK) >> OFFSET_BITS;

      // set present bit to zero
      subPage[subRowIndex] = 0;
      memMng.freePage(pageIdx);

      //printf("  FREE vp %u -> phys %u\n", virtualPage, pageIdx);
    }

    pagesCount = pagesChanged;
  }

  // returns addr of mapped page (not the page table)
  uint32_t *mapPage(uint32_t virtualPage, uint32_t *freePageArr, uint32_t &index) {
    auto mainPageTable = (uint32_t *)(m_MemStart + (m_PageTableRoot & ADDR_MASK));

    uint32_t mainPageRow = mainPageTable[virtualPage >> 10];

    auto *subPageTable = (uint32_t *)(m_MemStart + (mainPageRow & ADDR_MASK));
    // need to alloc new page table
    if (!(mainPageRow & BIT_PRESENT)) {
      uint32_t physical = freePageArr[index] << 12;

      memMng.allocPage(freePageArr[index]);
      uint32_t changedRow = (physical) | BIT_PRESENT | BIT_WRITE | BIT_USER;
      //printf("  NEW PAGE TABLE for dir %u -> phys %u\n", virtualPage >> 10, freePageArr[index]);

      subPageTable = (uint32_t *)(m_MemStart + physical);
      initializePageTable(subPageTable);

      mainPageTable[virtualPage >> 10] = changedRow;
      ++index;
    }

    uint32_t subTableIdx = virtualPage & 0x3FF;

    memMng.allocPage(freePageArr[index]);
    uint32_t changedRow = (freePageArr[index] << 12) | BIT_PRESENT | BIT_WRITE | BIT_USER;

    //printf("  MAP vp %u -> phys %u\n", virtualPage, freePageArr[index]);
    uint32_t *ret = (uint32_t *)(m_MemStart + (freePageArr[index] << 12));
    ++index;

    subPageTable[subTableIdx] = changedRow;
    return ret;
  }

  static uint32_t getAddrFromTableRow(uint32_t tableRow) { return (tableRow & ADDR_MASK) >> OFFSET_BITS; }

  uint32_t getNeededPages(uint32_t pagesNeeded) {
    uint32_t needed = pagesNeeded;
    auto mainPageTable = (uint32_t *)(m_MemStart + (m_PageTableRoot & ADDR_MASK));

    uint32_t lo = pagesCount >> 10;
    uint32_t hi = (pagesCount + pagesNeeded - 1) >> 10;

    for (uint32_t virt = lo; virt <= hi; ++virt) {

      uint32_t mainPageRow = mainPageTable[virt];

      if (!(mainPageRow & BIT_PRESENT)) {
        ++needed;
      }
    }
    return needed;
  }

  uint32_t *mainPageTable() { return (uint32_t *)(m_MemStart + (m_PageTableRoot & ADDR_MASK)); }

  uint32_t findFreeProcSlot() {
    for (uint32_t i = 0; i < 64; ++i) {
      if (!memMng.processes[i]) {
        return i;
      }
    }

    throw "Unexpected state";
  }
};

void *threadEntry(void *arg) {
  auto *context = (ThreadArg *)arg;

  context->entryPoint(context->proc, context->arg);

  // cleanup after itself
  uniqueLock lock(context->memMng.processMutex);

  // find "itself"
  for (int i = 0; i < 64; i++) {
    if (context->memMng.processes[i] == context->proc) {
      context->memMng.processes[i] = nullptr;
      break;
    }
  }

  delete context->proc;
  --context->memMng.activeProcCount;
  lock.unlock();

  // signal that a process died
  pthread_cond_signal(&context->memMng.processCv);

  delete context;
  return nullptr;
}

void MemManager::run(void *processArg, void (*mainProcess)(CCPU *, void *)) {
  pagesBitmap[0] = true;

  pagesBitmap[1] = true;
  initializePageTable((uint32_t *)(mem + 1 * CCPU::PAGE_SIZE));

  processes[0] = new Process((uint8_t *)mem, 1 * CCPU::PAGE_SIZE, *this);

  // start init func
  activeProcCount = 1;
  mainProcess(processes[0], processArg);

  uniqueLock lock(processMutex);
  --activeProcCount;

  while (activeProcCount > 0) {
    pthread_cond_wait(&processCv, &processMutex);
  }

  delete processes[0];

  //printf("Init ends.\n");
}

uint32_t MemManager::findFreePage() {
  if (!pagesBitmap[bitMapPtr % totalPages]) {
    return bitMapPtr % totalPages;
  }

  const uint32_t orig = bitMapPtr % totalPages;
  ++bitMapPtr;

  while (bitMapPtr % totalPages != orig) {
    if (!pagesBitmap[bitMapPtr % totalPages]) {
      return bitMapPtr % totalPages;
    }

    ++bitMapPtr;
  }

  return 0;
}

uint32_t *MemManager::findFreePages(uint32_t wantedPages) {
  uint32_t counter = 0;

  auto *freeArr = new uint32_t[wantedPages];

  if (!pagesBitmap[bitMapPtr % totalPages]) {
    freeArr[counter++] = bitMapPtr % totalPages;
  }

  const uint32_t orig = bitMapPtr % totalPages;
  ++bitMapPtr;

  while (counter != wantedPages) {

    if (bitMapPtr % totalPages == orig) {
      delete[] freeArr;
      return nullptr;
    }

    if (!pagesBitmap[bitMapPtr % totalPages]) {
      freeArr[counter++] = bitMapPtr % totalPages;
    }

    ++bitMapPtr;
  }

  //assert(counter == wantedPages);

  return freeArr;
}

// returns 0 if it fails (since zero is always taken by memMngr anyway)
void MemManager::allocPage(uint32_t pageIndex) { pagesBitmap[pageIndex] = true; }

void MemManager::freePage(uint32_t pageIndex) { pagesBitmap[pageIndex] = false; }

void MemMgr(void *mem, uint32_t totalPages, void *processArg, void (*mainProcess)(CCPU *, void *)) {
  MemManager man(mem, totalPages);

  man.run(processArg, mainProcess);
}
