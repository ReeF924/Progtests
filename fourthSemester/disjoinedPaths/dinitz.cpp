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

struct DeliverySolver {

  struct Edge {
    House from;
    House to;
    mutable unsigned capacity = 0;
    mutable unsigned flow = 0;
    size_t oppositeEdgeIdx = 0;

    void newEdge() const { ++capacity; }

    bool operator<(const Edge &e) const { return to < e.to; }

    unsigned availableFlow() const { return capacity - flow; }

    Edge(size_t retPosition, size_t direction, unsigned capacity, size_t oppositeEdgeIdx)
        : from(retPosition), to(direction), capacity(capacity), oppositeEdgeIdx(oppositeEdgeIdx) {}
  };
  using Vertex = std::vector<Edge>;

  DeliverySolver(unsigned houses, const std::vector<Road> &roads)
      : graph(std::vector<Vertex>(houses)), level(std::vector<int>(houses)), nextEdge(std::vector<unsigned>(houses)) {
    createGraph(roads);
    init();
  }

  std::vector<Vertex> graph;
  std::vector<unsigned> disjoinedPaths;
  std::vector<int> level;
  std::vector<unsigned> nextEdge;
  size_t sink = 0;

  void createGraph(const std::vector<Road> &roads) {
    std::map<std::pair<House, House>, unsigned> roadMap;

    size_t n = roads.size();
    for (size_t i = 0; i < n; ++i) {

      // don't care about these
      if (roads[i].first == roads[i].second) {
        continue;
      }

      std::pair<House, House> pair;

      if (roads[i].first < roads[i].second) {
        pair = {roads[i].second, roads[i].first};
      } else {
        pair = {roads[i].first, roads[i].second};
      }

      auto res = roadMap.emplace(pair, 0);
      res.first->second += 1;
    }

    for (const auto &road : roadMap) {
      addEdge(road.first.first, road.first.second, road.second);
    }
  }

  void addEdge(House k, House l, unsigned capacity) {
    size_t kSize = graph[k].size();
    size_t lSize = graph[l].size();

    graph[k].emplace_back(k, l, capacity, lSize);
    graph[l].emplace_back(l, k, capacity, kSize);
  }

  // just bfs with one more condition
  bool buildLeveledGraph() {
    // reinitialize the levels vector
    std::fill(level.begin(), level.end(), -1);

    std::queue<size_t> que;
    que.push(0);
    level[0] = 0;

    while (!que.empty()) {
      size_t curr = que.front();
      que.pop();

      for (const auto &e : graph[curr]) {

        // either is already on full capacity or doesn't go forward
        if (e.capacity - e.flow == 0 || level[e.to] != -1) {
          continue;
        }

        level[e.to] = level[e.from] + 1;
        que.push(e.to);
      }
    }

    // if the sink isn't found there is no paths to improve
    return level[sink] != -1;
  }

  struct Frame {
    House v;
    unsigned pushed;
  };

  void updateFlowsAlongPath(std::vector<Frame> &path, std::vector<size_t> &edges, unsigned flowIncrease) {

    for (size_t i = 0; i < edges.size(); ++i) {
      size_t v = path[i].v;

      Edge &e = graph[v][edges[i]];

      //assert(v == e.from);

      e.flow += flowIncrease;
      graph[e.to][e.oppositeEdgeIdx].flow -= flowIncrease;
    }
  }

  // now dfs
  unsigned improveFlow() {
    std::vector<Frame> path;
    path.reserve(graph.size());
    std::vector<size_t> edges;
    path.reserve(graph.size());

    path.push_back(Frame{.v = 0, .pushed = static_cast<unsigned>(-1)});

    while (!path.empty()) {
      Frame f = path.back();

      if (f.v == sink) {
        updateFlowsAlongPath(path, edges, path.back().pushed);
        return path.back().pushed;
      }

      bool foundPathLater = false;

      unsigned n = static_cast<unsigned>(graph[f.v].size());
      for (unsigned &i = nextEdge[f.v]; i < n; ++i) {

        const Edge &e = graph[f.v][i];

        if (level[e.to] == level[f.v] + 1 && e.availableFlow() > 0) {
          path.push_back(Frame{.v = e.to, .pushed = std::min(f.pushed, e.availableFlow())});
          edges.push_back(i);
          foundPathLater = true;
          break;
        }
      }

      if (!foundPathLater) {
        // don't wanna enter this vertex anymore, leads nowhere
        level[f.v] = -1;
        path.pop_back();
        if (!edges.empty()) {
          edges.pop_back();
        }
      }
    }

    return 0;
  }

  unsigned getDisjoinedPathsCount(size_t targetHouse) {
    unsigned maxFlow = 0;
    sink = targetHouse;

    while (buildLeveledGraph()) {
      std::fill(nextEdge.begin(), nextEdge.end(), 0);

      unsigned improvedBy = improveFlow();

      while (improvedBy > 0) {
        maxFlow += improvedBy;
        improvedBy = improveFlow();
      }
    }

    return maxFlow;
  }

  void resetGraph() {
    size_t n = graph.size();

    for (size_t i = 0; i < n; ++i) {
      for (size_t j = 0; j < graph[i].size(); ++j)
        graph[i][j].flow = 0;
    }
  }

  void init() {
    size_t n = graph.size();

    disjoinedPaths.push_back(static_cast<unsigned>(-1));

    for (size_t i = 1; i < n; ++i) {
      resetGraph();
      disjoinedPaths.push_back(getDisjoinedPathsCount(i));
    }
  }

  std::vector<bool> solve(unsigned policemen) const {
    std::vector<bool> ret;

    size_t n = disjoinedPaths.size();
    ret.reserve(n);

    for (size_t i = 0; i < n; ++i) {
      ret.push_back(policemen < disjoinedPaths[i]);
    }

    return ret;
  }
};

#ifndef __PROGTEST__
}

struct Test {
  unsigned houses;
  std::vector<Road> roads;
  std::vector<std::pair<unsigned, std::vector<bool>>> solution;
};

const std::vector<Test> examples = {
    Test{6,
         {{0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 4}, {0, 5}, {1, 5}},
         {
             {0, {1, 1, 1, 1, 1, 1}},
             {1, {1, 1, 1, 0, 0, 1}},
             {2, {1, 1, 0, 0, 0, 0}},
             {3, {1, 0, 0, 0, 0, 0}},
             {100, {1, 0, 0, 0, 0, 0}},
         }},
    //{ {: gen/examples-gen.inc }}
};

int main() {
  for (const auto &[h, r, l] : examples) {

    student_namespace::DeliverySolver solver(h, r);

    for (const auto &[p, ref] : l) {
      std::vector<bool> sol = solver.solve(p);
      assert(sol == ref);
    }
  }

  std::cout << "All tests passes" << std::endl;

  // runExtendedTests();
  std::cout << "All extended tests passes" << std::endl;
}

#endif
