#include "common.h"
#include "test_op.h"
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

static void seqTest0(CCPU *cpu, void *arg) {
  for (uint32_t i = 0; i <= 20; i++)
    checkResize(cpu, i);

  checkResize(cpu, 10);
  checkResize(cpu, 15);


  checkResize(cpu, 10);
  checkResize(cpu, 15);

  checkResize(cpu, 10);
  checkResize(cpu, 15);

  for (int i = 20; i >= 0; i--)
    checkResize(cpu, i);
}

static void seqTest1(CCPU *cpu, void *arg) {
  for (uint32_t i = 0; i <= 2000; i++)
    checkResize(cpu, i);

  checkResize(cpu, 5000);

  for (int i = 2000; i >= 0; i--)
    checkResize(cpu, i);
}

static void seqTest21(CCPU *cpu, void *arg) {
  checkResize(cpu, 10);
  rwiTest(cpu, 0, 10);

  checkResize(cpu, 7);
  rwiTest(cpu, 0, 7);
}

static void seqTest2(CCPU *cpu, void *arg) {
  checkResize(cpu, 1000);
  rwiTest(cpu, 0, 1000);

  checkResize(cpu, 2345);
  rwiTest(cpu, 0, 2345);

  checkResize(cpu, 789);
  rwiTest(cpu, 0, 789);
}

int main(void) {
  const int PAGES = 8 * 1024;

  // PAGES + extra 4KiB for alignment
  uint8_t *mem = new uint8_t[PAGES * CCPU::PAGE_SIZE + CCPU::PAGE_SIZE];

  // align to a mutiple of 4KiB
  uint8_t *memAligned = (uint8_t *)((((uintptr_t)mem) + CCPU::PAGE_SIZE - 1) & ~(uintptr_t)~CCPU::ADDR_MASK);

  testStart();
  MemMgr(memAligned, PAGES, NULL, seqTest0);
  testEnd("test #1");

  testStart();
  MemMgr(memAligned, PAGES, NULL, seqTest1);
  testEnd("test #1");

  testStart();
  MemMgr(memAligned, PAGES, NULL, seqTest21);
  testEnd("test #2");


  testStart();
  MemMgr(memAligned, PAGES, NULL, seqTest2);
  testEnd("test #2");


  delete[] mem;
  return 0;
}
