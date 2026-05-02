#ifndef __PROGTEST__
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

std::string rand_string(size_t len, auto &R) {
  constexpr std::string_view alphabet =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  std::string ret(len, alphabet[R() % alphabet.size()]);
  for (size_t i = len / 2; i < len; i++)
    ret[i] = alphabet[R() % alphabet.size()];

  return ret;
}

struct TestInfo {
  size_t words;
  size_t memory;
  unsigned succ, fail;
  unsigned max_wrong;
  uint64_t seed;
  size_t word_length = 200;
};

#endif

struct Hasher {
  static constexpr uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
  static constexpr uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
  static constexpr uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
  static constexpr uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
  static constexpr uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;
  size_t seed;

  Hasher(size_t seed) : seed(seed) {}
  Hasher() = default;

  uint64_t hash(std::string_view in) const {

    const char* ptr = in.data();
    // skip accumulators
    if (in.size() < 32) {
      uint64_t acc = seed + PRIME64_5;
      acc += in.size();

      return remainingOutput(acc, ptr, in.size());
    }

    uint64_t acc1 = seed + PRIME64_1 + PRIME64_2;
    uint64_t acc2 = seed + PRIME64_2;
    uint64_t acc3 = seed;
    uint64_t acc4 = seed - PRIME64_1;

    size_t counter = 0;

    size_t n = in.size() / 32;
    while (counter < n) {
      acc1 = round(acc1, ptr);
      acc2 = round(acc2, ptr + 8);
      acc3 = round(acc3, ptr + 16);
      acc4 = round(acc4, ptr + 24);

      ptr += 32 ;
      ++counter;
    }

    uint64_t acc = std::rotl(acc1, 1) + std::rotl(acc2, 7) +
                   std::rotl(acc3, 12) + std::rotl(acc4, 18);

    acc = mergeAccumulator(acc, acc1);
    acc = mergeAccumulator(acc, acc2);
    acc = mergeAccumulator(acc, acc3);
    acc = mergeAccumulator(acc, acc4);

    acc += in.size();

    size_t remLength = in.size() - counter * 32;

    return remainingOutput(acc, ptr, remLength);
  }

  uint64_t round(uint64_t acc, uint64_t lane) const {
    acc = acc + (lane * PRIME64_2);
    acc = std::rotl(acc, 31);
    return acc * PRIME64_1;
  }

  uint64_t round(uint64_t acc, const char* ptr) const {
    uint64_t lane = 0;
    std::memcpy(&lane, ptr, 8);

    return round(acc, lane);
  }

  uint64_t mergeAccumulator(uint64_t acc, uint64_t accUpd) const {
    acc = acc ^ round(0, accUpd);
    acc = acc * PRIME64_1;
    return acc + PRIME64_4;
  }

  uint64_t remainingOutput(uint64_t acc, const char *ptr,
                           size_t remLength) const {
    while (remLength >= 8) {
      uint64_t lane = 0;
      std::memcpy(&lane, ptr, 8);

      acc = acc ^ round(0, lane);
      acc = std::rotl(acc, 27) * PRIME64_1;
      acc += PRIME64_4;
      ptr += 8;
      remLength -= 8;
    }

    if (remLength >= 4) {
      uint32_t lane = 0;
      std::memcpy(&lane, ptr, 4);
      acc = acc ^ (lane * PRIME64_1);
      acc = std::rotl(acc, 23) * PRIME64_2;
      acc = acc + PRIME64_3;
      ptr += 4;
      remLength -= 4;
    }

    while (remLength >= 1) {
      char lane = *ptr;
      acc = acc ^ (lane * PRIME64_5);
      acc = std::rotl(acc, 11) & PRIME64_1;
      ptr += 1;
      remLength -= 1;
    }

    return acc;
  }
};

struct Bloom {
  static constexpr bool ENABLE_STD_HASH = false;
  size_t bitSize;
  std::vector<bool> data;
  char hashCount;
  size_t seed;
  Hasher hash1;
  Hasher hash2;

  Bloom(size_t expected_size, size_t allowed_memory)
      : bitSize(allowed_memory * 8), data(bitSize) {


    double hc = std::log(2) * static_cast<double>(allowed_memory) * 8 /
                static_cast<double>(expected_size);
    hc = std::ceil(hc);

    hc = hc > 20 ? 20 : hc;
    hc = hc < 2 ? 2 : hc;

    hashCount = static_cast<char>(hc);

    std::random_device rd1;

    seed = rd1() * 0xf1a3f0583bc01872;

    std::random_device rd2;
    hash1 = Hasher(rd2());
    std::random_device rd3;
    hash2 = Hasher(rd3());
  }

private:
  bool retTrue() { return true; }

  size_t splitMix64(size_t x) const {
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x = x ^ (x >> 31);
    return x;
  }

  std::pair<size_t, size_t> hashItem(const std::string_view &item) const {
    uint64_t h1 = hash1.hash(item);

    uint64_t h2 = hash2.hash(item);

    return {h1, h2};
  }

public:
  void insert(std::string_view s) {
    auto [h1, h2] = hashItem(s);

    for (char i = 0; i < hashCount; ++i) {
      size_t hash = (h1 + static_cast<size_t>(i) * h2) % bitSize;
      data[hash] = true;
    }
  }

  bool contains(std::string_view s) const {
    auto [h1, h2] = hashItem(s);

    for (char i = 0; i < hashCount; ++i) {
      size_t hash = (h1 + static_cast<size_t>(i) * h2) % bitSize;
      if (!data[hash])
        return false;
    }

    return true;
  }
};

#ifndef __PROGTEST__

#define CHECK(cond, ...)                                                       \
  do {                                                                         \
    if (cond)                                                                  \
      break;                                                                   \
    printf(__VA_ARGS__);                                                       \
    assert(0);                                                                 \
  } while (0)

#define CHECK_RES(r_res, s_res, wrong)                                         \
  do {                                                                         \
    if (r_res == s_res)                                                        \
      break;                                                                   \
    CHECK(r_res == 0, "False negative detected!");                             \
    wrong++;                                                                   \
  } while (0)

void test(const TestInfo info) {
  std::mt19937 R(info.seed);

  Bloom s(info.words, info.memory);
  std::vector<std::string> data;

  for (size_t i = 0; i < info.words; i++) {
    data.push_back(rand_string(info.word_length, R));
    s.insert(data.back());
  }

  std::ranges::sort(data);

  unsigned to_succ = info.succ, to_fail = info.fail;
  unsigned wrong = 0;
  while (to_succ || to_fail) {
    auto r = R() % (to_succ + to_fail);

    std::string w;
    if (r < to_succ) {
      to_succ--;
      w = data[R() % data.size()];
    } else {
      to_fail--;
      w = rand_string(info.word_length, R);
    }

    bool s_res = s.contains(w);
    bool r_res = std::ranges::binary_search(data, w);

    CHECK_RES(r_res, s_res, wrong);
  }

  printf("Wrong %u (limit %u; %.2lf %%)\n", wrong, info.max_wrong,
         wrong * 100. / info.max_wrong);
  CHECK(wrong <= info.max_wrong, "Wrong %u (limit %u; %.2lf %%)\n", wrong,
        info.max_wrong, wrong * 100. / info.max_wrong);
}

void example_tests(uint64_t seed) {
  test({.words = 20,
        .memory = 1'000'000,
        .succ = 10'000,
        .fail = 100'000,
        .max_wrong = 10,
        .seed = seed});

  test({.words = 6'000,
        .memory = 10'000,
        .succ = 200,
        .fail = 100'000,
        .max_wrong = 2 * 200,
        .seed = seed});

  test({.words = 6'000,
        .memory = 5'000,
        .succ = 200,
        .fail = 10'000,
        .max_wrong = 2 * 450,
        .seed = seed});
}

void example_tests_big(uint64_t seed) {
  test({.words = 600'000,
        .memory = 1'000'000,
        .succ = 200,
        .fail = 100'000,
        .max_wrong = 3 / 2 * 200,
        .seed = seed});
}

#undef CHECK_RESULT
#undef CHECK

int main() {
  Bloom b(10, 10);

  // cannot fail, bloom is empty
  assert(!b.contains("randomString"));

  b.insert("karel je borec");
  assert(b.contains("karel je borec"));

  b.insert("karel");
  b.insert("karel je");
  b.insert("je borec");
  assert(b.contains("karel"));
  assert(b.contains("karel je"));
  assert(b.contains("je borec"));
  assert(!b.contains("karel neco"));

  for (int i = 0; i < 2; i++)
    example_tests(i);
  for (int i = 0; i < 2; i++)
    example_tests_big(i);
}

#endif
