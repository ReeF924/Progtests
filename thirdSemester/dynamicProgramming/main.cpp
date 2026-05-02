#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <compare>

struct Vault {
  unsigned value;
  unsigned pebbles;
  std::vector<std::pair<unsigned, unsigned>> missing_connections;
};

struct UnlockingSequence {
  unsigned vault_id;
  std::vector<bool> moved_pebbles;
};

#endif

struct HeistRow
{
  HeistRow() = default;
  HeistRow(int idx, std::vector<bool> movedPebbles) : vaultId(idx), movedPebbles(std::move(movedPebbles)){}

  int vaultId = -1;
  std::vector<bool> movedPebbles = {};
  std::vector<unsigned> arr = {};
};

///pebbles count has to be positive and missing connections must not be empty
std::vector<std::pair<unsigned, std::vector<unsigned>>> componentVertexCount(const Vault& vault)
{
  //first value is visited, second is a vector of neighbour vertexes
  std::vector<std::pair<bool, std::vector<unsigned>>> edges(vault.pebbles);

  //create graph out of missing connections
  for (auto mc: vault.missing_connections)
  {
    edges[mc.first].second.push_back(mc.second);
    edges[mc.second].second.push_back(mc.first);
  }

  std::queue<unsigned> que;
  std::vector<std::pair<unsigned, std::vector<unsigned>>> numbers;

  for (unsigned i = 0; i < vault.pebbles; ++i)
  {
    //already visited
    if (edges[i].first)
      continue;


    numbers.push_back({0, {}});
    que.push(i);
    edges[i].first = true;

    //number of vertexes in this component
    while (!que.empty())
    {
      auto curr = que.front();
      que.pop();
      numbers.back().second.push_back(curr);

      ++numbers.back().first;

      for (size_t j = 0; j < edges[curr].second.size(); ++j)
      {
        const unsigned neighbour = edges[curr].second[j];

        if (edges[neighbour].first)
          continue;

        que.push(neighbour);
        //set visited on the neighbour
        edges[neighbour].first = true;
      }
    }
    if (numbers.back().first > vault.pebbles / 2)
    {
      return {};
    }
  }

  return numbers;
}

std::vector<bool> getPebblesToMove(std::vector<std::vector<bool>>& table,
  std::vector<std::pair<unsigned, std::vector<unsigned>>>& nums,
  size_t idx, unsigned pebblesCount)
{
  std::vector<bool> ret(pebblesCount);

  unsigned sum = pebblesCount / 2;

  //is called after the iteration that found the correct sum
  for (; idx != 0; --idx)
  {
    //number wasn't used
    if (table[idx][sum] == table[idx-1][sum])
    {
      continue;
    }

    sum -= nums[idx-1].first;
    for (auto pebble: nums[idx-1].second)
    {
      ret[pebble] = true;
    }
    if (sum == 0)
    {
      break;
    }
  }
  return ret;
}

std::pair<bool, std::vector<bool>> vaultOpenable(const Vault& vault)
{
  //odd number of pebbles => vault can't be opened
  if (vault.pebbles % 2)
    return {false, {}};
  if (vault.pebbles == 0)
    return {true, {}};

  std::vector<bool> ret(vault.pebbles);
  const unsigned target = vault.pebbles / 2;

  //no missing connections => always possible, just move any pebbles
  if (vault.missing_connections.empty())
  {
    for (unsigned p = 0; p < target; ++p)
      ret[p] = true;

    return {true, std::move(ret)};
  }

  //first is the count of vertexes in a component, the .second is the idx of them
  std::vector<std::pair<unsigned, std::vector<unsigned>>> nums = componentVertexCount(vault);


  std::vector<std::vector<bool>> table(nums.size() + 1);
  table[0].resize(target + 1);


  //always reachable
  table[0][0] = true;

  size_t n = 1;
  for (; n <= nums.size(); ++n)
  {
    size_t numIdx = n - 1;
    table[numIdx].reserve(target + 1);

    for (unsigned i = 0; i < nums[numIdx].first; ++i)
    {
      table[n].push_back(table[n-1][i]);
    }

    for (unsigned i = nums[numIdx].first; i <= target; ++i)
    {
      //if is already reachable keep it that way, if not check whether I can't get there with this num
      table[n].push_back(table[n-1][i] || table[n-1][i - nums[numIdx].first]);
    }

    //if I can reach the target already, no reason to continue
    if (table[n][target])
    {
      return {true, std::move(getPebblesToMove(table, nums, n, vault.pebbles))};
    }
  }

  return {false, {}};
}

std::vector<UnlockingSequence> getIncludedItems(const std::vector<Vault>& vaults, std::vector<HeistRow>& table, unsigned transitionTime)
{
  std::vector<UnlockingSequence> ret;

  int currTimeSpent = static_cast<int>(table[0].arr.size() - 1);

  //start at last row of vec go until first non-empty row
  for (int row = static_cast<int>(table.size() - 1); row != 0; --row)
  {
    //item wasn't included
    if (table[row].arr[currTimeSpent] == table[row - 1].arr[currTimeSpent])
    {
      continue;
    }

    ret.push_back({.vault_id = static_cast<unsigned>(table[row].vaultId), .moved_pebbles = std::move(table[row].movedPebbles)});

    //don't have to care about the last
    currTimeSpent -= static_cast<int>(vaults[table[row].vaultId].pebbles / 2 + transitionTime);

    //must have been the first element I just subtracted
    if (currTimeSpent < 0)
    {
      break;
    }
  }

  return ret;
}

std::vector<UnlockingSequence> plan_heist(const std::vector<Vault>& vaults, unsigned transition_time, unsigned max_time)
{
  if (vaults.empty())
    return {};

  //upper boundary is included
  ++max_time;

  //2D table, rows are vaults, columns is time
  //the .first of the std::pair is vault index (to ignore vaults that cannot be opened)
  std::vector<HeistRow> table;
  table.reserve(vaults.size() + 1);

  //init first row with zeros
  table.emplace_back();
  table.back().arr.resize(max_time);

  const unsigned n = vaults.size();
  int tableIdx = 0;

  for (unsigned vIdx = 0; vIdx < n; ++vIdx)
  {
    const unsigned timeSpent = vaults[vIdx].pebbles / 2;

    //skip vaults that I can't open or take too much time to open
    if (timeSpent >= max_time)
    {
      continue;
    }

    std::pair<bool, std::vector<bool>> pebblesToMove = vaultOpenable(vaults[vIdx]);

    if (!pebblesToMove.first)
    {
      continue;
    }

    //push back vaultIdx and the vector
    ++tableIdx;
    table.emplace_back(vIdx, std::move(pebblesToMove.second));
    table.back().arr.reserve(max_time);

    //copy values, cannot add the new item yet
    for (unsigned t = 0; t < timeSpent; ++t)
    {
      table[tableIdx].arr.push_back(table[tableIdx-1].arr[t]);
    }

    //now compare if adding is better or not

    //can only be added as first vault
    const size_t upBnd = std::min(timeSpent + transition_time, max_time);
    for (unsigned t = timeSpent; t < upBnd; ++t)
    {
      //so check if the value of this vault is higher than the sum above
      table[tableIdx].arr.push_back(std::max(
          vaults[vIdx].value,
          table[tableIdx-1].arr[t]
        ));
    }

    for (unsigned t = upBnd; t < max_time; ++t)
    {
      //either add the previous max price for the weight or add this item
      //w - item.weight can't be negative, because w >= item.weight
      table[tableIdx].arr.push_back(std::max(
        table[tableIdx-1].arr[t - timeSpent - transition_time] + vaults[vIdx].value,
        table[tableIdx-1].arr[t]
        ));
    }
  }

  return getIncludedItems(vaults, table, transition_time);
}

#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
  using std::runtime_error::runtime_error;
};

#define CHECK(cond, msg) do { \
    if (!(cond)) throw TestFailed(msg); \
  } while (0)

void check_unlocking_sequence(
  unsigned pebbles,
  const std::vector<std::pair<unsigned, unsigned>>& missing_connections,
  const std::vector<bool>& moved
) {
  CHECK(moved.size() == pebbles, "Solution has wrong size.\n");

  size_t moved_cnt = 0;
  for (bool p : moved) moved_cnt += p;
  CHECK(2*moved_cnt == pebbles,
    "Exactly half of the pebbles must be moved.\n");

  for (auto [ u, v ] : missing_connections) CHECK(moved[u] == moved[v],
    "Pebble not connected with all on other side.\n");
}

void check_solution(
  const std::vector<UnlockingSequence>& solution,
  unsigned expected_value,
  const std::vector<Vault>& vaults,
  unsigned transition_time,
  unsigned max_time
) {
  unsigned time = 0, value = 0;
  std::vector<bool> robbed(vaults.size(), false);

  for (size_t i = 0; i < solution.size(); i++) {
    const auto& [ id, moved ] = solution[i];

    CHECK(id < vaults.size(), "Id is out of range.\n");
    CHECK(!robbed[id], "Robbed same vault twice.\n");
    robbed[id] = true;
    
    const auto& vault = vaults[id];
    value += vault.value;

    if (i != 0) time += transition_time;
    time += vault.pebbles / 2;
    CHECK(time <= max_time, "Run out of time.\n");

    check_unlocking_sequence(vault.pebbles, vault.missing_connections, moved);
  }

  CHECK(value == expected_value, "Total value mismatch.\n");
}


struct Test {
  unsigned expected_value;
  unsigned max_time;
  unsigned transition_time;
  std::vector<Vault> vaults;
};

inline const std::vector<Test> TESTS = {
  //1
  Test{
    .expected_value = 3010, .max_time = 3, .transition_time = 8,
    .vaults = {
      { .value = 3010, .pebbles = 6, .missing_connections = { {3,4}, {0,1}, {4,5}, {5,3}, } },
      { .value = 3072, .pebbles = 6, .missing_connections = { {2,1}, {1,3}, {0,1}, {0,3}, {4,5}, {2,3}, } },
      { .value = 5069, .pebbles = 10, .missing_connections = { {7,2}, {3,4}, {0,1}, {8,4}, {1,2}, {8,3}, {7,0}, {5,6}, {9,5}, {9,6}, } },
      { .value = 2061, .pebbles = 4, .missing_connections = { {3,0}, {2,1}, {0,2}, {1,3}, } },
    }
  },
  //2
  Test{
    .expected_value = 6208, .max_time = 13, .transition_time = 12,
    .vaults = {
      { .value = 6011, .pebbles = 12, .missing_connections = { {1,5}, {2,4}, {5,10}, {1,10}, {0,3}, {8,3}, {8,0}, {9,8}, {2,6}, {3,9}, {0,9}, {4,6}, {11,7}, } },
      { .value = 2056, .pebbles = 4, .missing_connections = { {1,0}, {2,0}, {2,1}, } },
      { .value = 5885, .pebbles = 12, .missing_connections = { {1,6}, {3,7}, {1,0}, {2,9}, {9,8}, {2,8}, {5,7}, {11,4}, {10,1}, {5,3}, {0,10}, } },
      { .value = 5818, .pebbles = 12, .missing_connections = { {9,0}, {7,1}, {6,4}, {8,6}, {4,2}, {11,5}, {5,3}, {9,7}, {8,4}, {2,8}, {10,11}, {5,10}, {10,3}, {9,1}, } },
      { .value = 4880, .pebbles = 10, .missing_connections = { {7,3}, {4,1}, {9,2}, {6,9}, {2,6}, {5,0}, {8,4}, } },
      { .value = 5233, .pebbles = 10, .missing_connections = { {0,2}, {4,5}, {8,3}, {9,7}, {7,1}, {6,3}, {6,8}, } },
      { .value = 6208, .pebbles = 12, .missing_connections = { {1,7}, {3,4}, {10,7}, {0,3}, {8,2}, {5,1}, {9,11}, {0,6}, {6,3}, {10,1}, {0,4}, } },
      { .value = 4182, .pebbles = 8, .missing_connections = { {5,7}, {7,4}, {4,5}, {1,0}, {5,6}, {3,1}, {6,4}, {0,3}, {6,7}, } },
    }
  },
};

int main() {
  int ok = 0, fail = 0;

  int counter = 1;
  for (auto t : TESTS) {
    try {
      auto sol = plan_heist(t.vaults, t.transition_time, t.max_time);
      check_solution(sol, t.expected_value, t.vaults, t.transition_time, t.max_time);
      ok++;
    } catch (const TestFailed& tf) {
      fail++;
      std::cout << counter << ": " << tf.what();
    }
    ++counter;
  }

  if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
  else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


