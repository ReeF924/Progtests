#include <cstddef>
#ifndef __PROGTEST__
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <compare>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <ranges>
#include <set>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

using House = size_t;
using Road = std::pair<House, House>;

namespace student_namespace {
#endif

struct Edge {

  size_t retPosition;
  size_t direction;
  size_t totalDemand = 0;

  Edge(size_t retPosition, size_t direction) : retPosition(retPosition), direction(direction) {}
};

struct Vertex {
  // enum State { NotFound, Open, Closed };

  size_t id;
  size_t demand;
  int in = -1;
  std::vector<Edge> edges;

  Vertex(size_t id, size_t demand) : id(id), demand(demand) {}
};


std::vector<Vertex> createGraph(const std::vector<unsigned> &houses, const std::vector<Road> &roads) {

  std::vector<Vertex> g;
  size_t n = houses.size();
  g.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    g.push_back(Vertex(i, houses[i]));
  }

  n = roads.size();

  for (size_t i = 0; i < n; ++i) {

    // don't care about these but for easier output
    if (roads[i].first == roads[i].second) {
      g[roads[i].first].edges.push_back(Edge(i, roads[i].second));
      continue;
    }

    g[roads[i].first].edges.push_back(Edge(i, roads[i].second));
    g[roads[i].second].edges.push_back(Edge(i, roads[i].first));
  }

  return g;
}

size_t dfs(std::vector<Vertex> &g, size_t curr, int &in, size_t parent, int &low) {

  size_t sumDemand = g[curr].demand;

  // low is current in (that is this counter)
  g[curr].in = in++;
  // g[curr].state = Vertex::Open;

  // chceme zjistit, jestli je tato hrana most
  // pokud ano, na vystupu 0, jinak suma podstromu
  for (Edge &e : g[curr].edges) {

    // prave jsem po tehle hrane prisel, nebo jsem po ni uz sel
    if (e.direction == parent) {
      continue;
    }

    // zpetna hrana, pokud pomuze, tak upravime low (muze techto hran byt vic)
    // zpetna hrana nutne neni most
    // postara se i o smycky
    if (g[e.direction].in != -1) {
      low = g[e.direction].in < low ? g[e.direction].in : low;
      continue;
    }

    // stromova
    // now in has been incremented for curr, so it in(curr) = in - 1
    int lowSnapshot = in;
    int subTreeLow = in;

    size_t demand = dfs(g, e.direction, in, curr, subTreeLow);

    // tato hrana je most => upravit vysledek
    if (subTreeLow == lowSnapshot) {
      e.totalDemand = demand;
    } 
    else {
      low = subTreeLow < low ? subTreeLow : low;
    }

    sumDemand += demand;
  }

  return sumDemand;
}

std::vector<unsigned> rate_roads(const std::vector<unsigned> &houses, const std::vector<Road> &roads) {

  assert(!houses.empty());

  auto g = createGraph(houses, roads);

  int counter = 0;
  int low = 0;
  // start at vertex
  dfs(g, 0, counter, 0, low);
  std::vector<unsigned> result(roads.size());

  for (Vertex &v : g) {
    if (v.in == -1) {
      continue;
    }

    for (Edge &e : v.edges) {
      result[e.retPosition] = result[e.retPosition] < e.totalDemand ? e.totalDemand : result[e.retPosition];
    }
  }

  return result;
}

#ifndef __PROGTEST__
}

struct Test {
  std::vector<unsigned> houses;
  std::vector<Road> roads;
  std::vector<unsigned> solution;
};

const std::vector<Test> examples = {
    Test{{0, 5, 3, 1, 8, 14}, {{0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 4}, {0, 5}, {1, 5}}, {0, 0, 0, 9, 8, 0, 0}},
    Test{
        {
            765,
            184,
            298,
            693,
            109,
            809,
        },
        {
            {5, 4},
            {5, 1},
            {3, 4},
            {4, 1},
            {3, 0},
            {0, 4},
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
        },
    },
    Test{
        {
            960,
            487,
            653,
            168,
            211,
            18,
            760,
        },
        {
            {1, 0},
            {6, 3},
            {1, 2},
            {0, 4},
            {1, 3},
            {0, 5},
        },
        {
            2068,
            760,
            653,
            211,
            928,
            18,
        },
    },
    Test{
        {
            930,
            692,
            93,
            628,
            183,
            245,
            947,
            447,
        },
        {
            {3, 4},
            {4, 6},
            {7, 3},
            {0, 2},
            {2, 3},
            {0, 4},
            {5, 2},
            {1, 0},
        },
        {
            0,
            947,
            447,
            0,
            0,
            0,
            245,
            692,
        },
    },
    Test{
        {
            772,
            7,
            756,
            842,
            644,
            151,
            941,
            295,
            609,
        },
        {
            {4, 5},
            {0, 6},
            {3, 2},
            {7, 8},
            {6, 7},
            {0, 5},
            {2, 8},
            {4, 1},
        },
        {
            651,
            3443,
            842,
            2207,
            2502,
            802,
            1598,
            7,
        },
    },
    Test{
        {
            379,
            464,
            989,
            390,
            593,
            749,
            884,
            222,
            629,
            139,
        },
        {
            {8, 0},
            {0, 3},
            {0, 1},
            {8, 2},
            {5, 4},
            {4, 7},
            {4, 3},
            {9, 5},
            {6, 0},
        },
        {
            1618,
            2093,
            464,
            989,
            888,
            222,
            1703,
            139,
            884,
        },
    },
};

int main() {
  for (const auto &[h, r, s] : examples) {
    std::vector<unsigned> sol = student_namespace::rate_roads(h, r);
    assert(sol == s);
  }

  std::cout << "All tests passes" << std::endl;
}

#endif
