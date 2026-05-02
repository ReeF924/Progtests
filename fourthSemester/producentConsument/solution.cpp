#ifndef __PROGTEST__
#include "progtest_solver.h"
#include "sample_tester.h"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cfloat>
#include <chrono>
#include <climits>
#include <cmath>
#include <compare>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <stack>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#endif /* __PROGTEST__ */

uint32_t getFragmentId(uint64_t fragment) { return static_cast<uint32_t>((fragment >> 42) & 0x3FFFF); }
uint32_t getFragCnt(uint64_t fragment) {
  return static_cast<uint32_t>((fragment >> 37) & 0x1F);
  // return static_cast<uint32_t>((fragment & SHIFT_FRAGMENT_CNT) >> 37);
}

struct TempContainers {
  std::vector<AReceiver> receivers;
  std::vector<ATransmitter> transmitters;
};
struct ThreadStruct {
  std::vector<std::thread> receiverThreads;
  std::vector<std::thread> workerThreads;
  std::vector<std::thread> transmitterThreads;
};

struct fragmentBlock {
  uint32_t fragCnt;
  std::vector<uint64_t> fragments;

  bool valid = true;
  fragmentBlock(uint32_t fragCnt) : fragCnt(fragCnt) {}

  bool pushFragment(uint64_t fragment) {
    if (!valid) {
      return false;
    }

    if (getFragCnt(fragment) != fragCnt) {
      return false;
    }

    fragments.push_back(fragment);
    return true;
  }

  bool full() { return fragCnt == fragments.size() - 1; }
};

class CSentinelHacker {
  using sint = uint64_t;
  using tint = uint32_t;

  struct CountData {
    tint id;
    const uint8_t *data;
    size_t bits;

    CountData() {}
    CountData(tint id, const uint8_t *data, size_t bits) : id(id), data(data), bits(bits) {}
  };

  struct MessageStatus {
    tint totalToCount = 0;
    tint timesCounted = 0;
    CBigInt max = 0;
  };

  TempContainers cont;
  ThreadStruct threadStr;

  std::unordered_map<tint, fragmentBlock> fragmentMap;
  std::mutex fragmentMapMutex;

  std::queue<AMsgSerializer> taskQueue;
  std::mutex taskQueueMutex;
  std::condition_variable taskQueueEmptyCv;

  AMsgSerializer serializer;
  std::mutex serializerMutex;

  std::unordered_map<tint, MessageStatus> activeMessages;
  std::mutex activeMessagesMutex;

  std::queue<std::pair<tint, std::optional<CBigInt>>> resultsQueue;
  std::mutex resultsQueueMutex;
  std::condition_variable resultsQueueEmptyCv;

  std::atomic_bool workersFinish;
  std::atomic_bool transmitterFinish;

public:
  static bool seqSolve(const std::vector<uint64_t> &fragments, CBigInt &res) {
    const AMsgSerializer ms = createMsgSerializer();

    if (fragments.empty()) {
      return false;
    }

    tint id = getFragmentId(fragments[0]);
    tint fragCnt = getFragCnt(fragments[0]);

    if (fragCnt != fragments.size() - 1) {
      return false;
    }

    // check if all fragCounts are the same
    for (sint fragment : fragments) {
      if (fragCnt != getFragCnt(fragment)) {
        return false;
      }
      if (id != getFragmentId(fragment)) {
        return false;
      }
    }

    res = 0;
    bool foundComb = false;
    bool success = ms->addProblem(
        fragments,
        [&res](tint msgId, const uint8_t data[], size_t bits) {
          CBigInt cnt = countExpressions(data, bits);

          if (res < cnt) {
            res = cnt;
          }
        },
        [&foundComb](tint messagesCount) { foundComb = messagesCount != 0; });

    if (!success) {
      return false;
    }

    success = ms->solve();

    return success && foundComb;
  }

  void addTransmitter(ATransmitter x) { cont.transmitters.push_back(x); }
  void addReceiver(AReceiver x) { cont.receivers.push_back(x); }

  void addFragment(sint fragment) {
    tint id = getFragmentId(fragment);
    tint fragCnt = getFragCnt(fragment);

    //std::cout << "Add fragment: " << id << std::endl;
    std::unique_lock fragmentLock(fragmentMapMutex);

    auto it = fragmentMap.emplace(id, fragmentBlock(fragCnt)).first;

    bool res = it->second.pushFragment(fragment);

    if(!res){
      //std::cout << "invalid fragment, add inv result: " << id << std::endl;
      std::unique_lock resLock(resultsQueueMutex);
      resultsQueue.emplace(id, std::nullopt);
      resLock.unlock();
      resultsQueueEmptyCv.notify_one();
      return;
    }

    // fragment can be added
    if (it->second.full()) {
      //  create a new task
      std::vector<sint> fragsToAdd = std::move(it->second.fragments);

      fragmentMap.erase(it);
      fragmentLock.unlock();

      std::lock_guard serializerLock(serializerMutex);

      //std::cout << "Fragments complete, add problem: " << id << std::endl;
      AMsgSerializer ser = serializer;
      serializer->addProblem(
          fragsToAdd,
          [this](tint msgId, const uint8_t data[], size_t bitCount) {
            CBigInt c = countExpressions(data, bitCount);
            //std::cout << "counted expr: " << msgId << std::endl;

            std::unique_lock activeMessagesLock(activeMessagesMutex);

            MessageStatus ms;
            auto it = activeMessages.emplace(msgId, ms).first;

            ++it->second.timesCounted;

            if (it->second.max < c) {
              it->second.max = std::move(c);
            }

            // counted the last message option
            if (it->second.timesCounted == it->second.totalToCount) {
              //std::cout << "upload result" << msgId << std::endl;
              CBigInt max = std::move(it->second.max);

              activeMessages.erase(it);
              activeMessagesLock.unlock();

              std::unique_lock resultsQueueLock(resultsQueueMutex);

              resultsQueue.emplace(msgId, std::move(max));
              resultsQueueLock.unlock();
              resultsQueueEmptyCv.notify_one();
            }
          },
          [this, id](tint totalMessages) {
            //std::cout << "finish: " << id << std::endl;

            // foundFn never called, it's not in activeMessages at all
            if (totalMessages == 0) {
              std::lock_guard resultsQueueLock(resultsQueueMutex);

              resultsQueue.emplace(id, std::nullopt);
              return;
            }

            std::unique_lock activeMessagesLock(activeMessagesMutex);
            auto it = activeMessages.emplace(id, MessageStatus()).first;

            it->second.totalToCount = totalMessages;

            if (it->second.timesCounted == totalMessages) {
              // it has already been counted, upload result
              CBigInt max = std::move(it->second.max);
              //std::cout << "upload result" << id << std::endl;

              activeMessages.erase(it);
              activeMessagesLock.unlock();

              std::unique_lock resultsQueueLock(resultsQueueMutex);

              resultsQueue.emplace(id, std::move(max));
              resultsQueueLock.unlock();
              resultsQueueEmptyCv.notify_one();
            }
          });

      // if serializer is full create a new one and then add the problem to the
      // new serializer (should never be zero right away, so I can check it only afterwards)
      if (!serializer->hasFreeCapacity()) {
        //std::cout << "Solver full" << std::endl;
        tint n = serializer->totalThreads();

        std::unique_lock taskQueLock(taskQueueMutex);

        for (tint i = 0; i < n; ++i) {
          taskQueue.emplace(serializer);
        }

        taskQueLock.unlock();
        taskQueueEmptyCv.notify_all();

        serializer = createMsgSerializer();
        //std::cout << "New serializer" << std::endl;
      }
    }
  }

  void start(unsigned thrCount) {
    // init first serializer
    std::unique_lock serLock(serializerMutex);
    serializer = createMsgSerializer();
    serLock.unlock();

    for (AReceiver &rec : cont.receivers) {
      threadStr.receiverThreads.push_back(std::thread(&CSentinelHacker::milkReceivers, this, rec));
    }

    for (unsigned i = 0; i < thrCount; ++i) {
      threadStr.workerThreads.push_back(std::thread(&CSentinelHacker::workerThread, this));
    }

    for (ATransmitter &tran : cont.transmitters) {
      threadStr.transmitterThreads.push_back(std::thread(&CSentinelHacker::submitMessages, this, tran));
    }
  }
  void stop() {
    //std::cout << "waiting for receiver threads" << std::endl;
    //  will stop by themselves
    for (auto &t : threadStr.receiverThreads) {
      t.join();
    }

    //std::cout << "joined receiver threads" << std::endl;

    //  no more fragments are gonna be added, so all left are incomplete
    {
      //std::cout << "adding incomplete" << std::endl;
      std::lock_guard resultsLock(resultsQueueMutex);

      for (auto &frag : fragmentMap) {
        //std::cout << "incomplete message addToResults: " << frag.first << std::endl;
        resultsQueue.emplace(frag.first, std::nullopt);
      }
    }
    resultsQueueEmptyCv.notify_all();

    std::unique_lock serLock(serializerMutex);
    //std::cout << "adding last solve" << std::endl;

    tint n = serializer->totalThreads();

    std::unique_lock taskQueLock(taskQueueMutex);

    for (tint i = 0; i < n; ++i) {
      taskQueue.emplace(serializer);
    }

    taskQueLock.unlock();

    serializer = nullptr;

    serLock.unlock();

    workersFinish = true;
    taskQueueEmptyCv.notify_all();

    // atomic bool ends all worker threads
    //std::cout << "waiting for workers to finish" << std::endl;

    for (auto &t : threadStr.workerThreads) {
      t.join();
    }

    //std::cout << "workers done, transmitters now" << std::endl;

    transmitterFinish = true;
    resultsQueueEmptyCv.notify_all();
    //  all worker threads finished => no more callbacks => no more results
    for (auto &t : threadStr.transmitterThreads) {
      t.join();
    }

    //std::cout << "workers done, transmitters now" << std::endl;
  }

private:
  void milkReceivers(AReceiver rec) {
    sint frag;

    while (rec->recv(frag)) {
      addFragment(frag);
    }
  }

  void workerThread() {
    while (true) {
      std::unique_lock taskQueueLock(taskQueueMutex);
      taskQueueEmptyCv.wait(taskQueueLock, [this]() { return (!taskQueue.empty()) || workersFinish; });

      // everything finished
      if (workersFinish && taskQueue.empty()) {
        //std::cout << "Worker died" << std::endl;
        return;
      }

      auto serializer = taskQueue.front();
      taskQueue.pop();

      taskQueueLock.unlock();

      //std::cout << "Join solve" << std::endl;
      serializer->solve();
    }
  }

  void submitMessages(ATransmitter t) {
    while (true) {
      std::unique_lock resultsQueLock(resultsQueueMutex);

      resultsQueueEmptyCv.wait(resultsQueLock, [this]() { return !resultsQueue.empty() || transmitterFinish; });

      if (transmitterFinish && resultsQueue.empty()) {
        //std::cout << "Transmitter died" << std::endl;
        return;
      }

      tint id = resultsQueue.front().first;

      if (!resultsQueue.front().second.has_value()) {
        //std::cout << "Upload incomplete result: " << id << ", transmitter: " << t << std::endl;
        resultsQueue.pop();

        resultsQueLock.unlock();

        t->incomplete(id);
        continue;
      }

      CBigInt res = std::move(resultsQueue.front().second.value());
      resultsQueue.pop();
      resultsQueLock.unlock();

      //std::cout << "Upload result: " << id << ", transmitter: " << t << std::endl;
      t->send(id, res);
    }
  }
};

#ifndef __PROGTEST__
int main() {
  using namespace std::placeholders;
  msgSerializerLimits(100, 1, 1, 1, 1); // friendly CMsgSerializer instances in sequential tests
                                        //
  for (const auto &x : g_TestSets) {
    CBigInt res;
    assert(CSentinelHacker::seqSolve(x.m_Fragments, res));
    assert(CBigInt(x.m_Result) == res);
  }
  msgSerializerLimits(4, 3, 5, 2, 2); // more exciting boundaries for real runs
                                      //

  CSentinelHacker test;
  auto trans = std::make_shared<CExampleTransmitter>();
  auto trans2 = std::make_shared<CExampleTransmitter>();
  AReceiver recv = std::make_shared<CExampleReceiver>(
      std::initializer_list<uint64_t>{0x508e000072ba, 0x508a000004a1, 0x788a0000058c, 0x246700000092});

  AReceiver recv2 = std::make_shared<CExampleReceiver>(
      std::initializer_list<uint64_t>{0x00000073103172ba, 0x00000073103172bb, 0x00000073103172bc, 0x00000073103172bd, 0x1111111111111111});

  //std::cout << getFragmentId(0x00000073103172ba) << " frag id" << std::endl;
  //std::cout << getFragCnt(0x00000073103172ba) << " frag cnt" << std::endl;

  test.addTransmitter(trans);
  test.addTransmitter(trans2);
  test.addReceiver(recv);
  test.addReceiver(recv2);
  test.start(9);

  static std::initializer_list<uint64_t> t1Data = {0x247300061fa2, 0x246d00003977, 0x5c8e000029aa, 0x5c890000009b};
  std::thread t1(fragmentSender, std::bind(&CSentinelHacker::addFragment, &test, _1), t1Data);

  static std::initializer_list<uint64_t> t2Data = {0x788d000036c6, 0x788e00002ab0, 0x508a0000036c, 0x246b00000e2b};
  std::thread t2(fragmentSender, bind(&CSentinelHacker::addFragment, &test, _1), t2Data);
  fragmentSender(std::bind(&CSentinelHacker::addFragment, &test, _1),
                 std::initializer_list<uint64_t>{0x508d0000007f, 0x5c8b00000aab, 0x788e00007d7d, 0x508d00002f0b,
                                                 0x7893000e6648, 0x5c8f00009f2d});
  t1.join();
  t2.join();
  test.stop();

  assert(trans->totalSent() + trans2->totalSent() == 2);
  assert(trans->totalIncomplete() + trans2->totalIncomplete() == 4);

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
