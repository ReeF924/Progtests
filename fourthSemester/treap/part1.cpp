#ifndef __PROGTEST__
#include <array>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <type_traits>
#include <utility>

// We use std::set as a reference to check our implementation.
// It is not available in progtest :)
#include <set>

template <typename T> struct Ref {
  size_t size() const { return _data.size(); }
  const T *find(const T &value) const {
    auto it = _data.find(value);
    if (it == _data.end())
      return nullptr;
    return &*it;
  }
  bool insert(const T &value) { return _data.insert(value).second; }
  bool erase(const T &value) { return _data.erase(value); }

  auto begin() const { return _data.begin(); }
  auto end() const { return _data.end(); }

private:
  std::set<T> _data;
};

#endif

namespace config {
// Disable if your implementation does not have parent pointers
inline constexpr bool PARENT_POINTERS = true;
} // namespace config

template <typename T> struct Treap {
  struct Node {
    T value;
    double weight;
    Node *parent;
    Node *left = nullptr;
    Node *right = nullptr;

    Node(T value, Node *parent, double priority)
        : value(std::move(value)), weight(priority), parent(parent) {}

    Node(T value, Node *parent) : value(std::move(value)), parent(parent) {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      static std::uniform_real_distribution<double> dis(0, 1);

      weight = dis(gen);
    }
  };

  Node *root = nullptr;
  size_t count = 0;

  ~Treap() {
    Node *current = root;
    Node *last = nullptr;

    while (current != nullptr) {
      if (last == current->parent) {
        if (current->left != nullptr) {
          last = current;
          current = current->left;
          continue;
        }
        if (current->right != nullptr) {
          last = current;
          current = current->right;
          continue;
        }

        // found leaf, delete it
        last = current;
        Node *parent = current->parent;
        delete current;
        current = parent;
      } else if (last == current->left) {
        if (current->right != nullptr) {
          // coming up from left subtree
          last = current;
          current = current->right;
          continue;
        }

        // No right child, delete current
        last = current;
        Node *parent = current->parent;
        delete current;
        current = parent;
      } else if (last == current->right) {
        // Coming up from right subtree
        last = current;
        Node *parent = current->parent;
        delete current;
        current = parent;
      }
    }
  }

  size_t size() const { return count; }
  const T *find(const T &value) const {
    Node *curr = root;

    while (curr) {
      // go left
      if (value < curr->value) {
        curr = curr->left;
        continue;
      }

      // go right
      if (curr->value < value) {
        curr = curr->right;
        continue;
      }

      // the value isn't smaller nor bigger than the curr.value => element found
      return &curr->value;
    }

    return nullptr;
  }

  bool insert(T value) { return insert(new Node(std::move(value), nullptr)); }

  bool insert(Node *node) {
    auto splitInfo = treapSplit(root, node->value);

    // element already exists
    if (splitInfo.found) {
      root = treapJoin(splitInfo.t1, splitInfo.t2);
      delete node;
      return false;
    }

    root = treapJoin(treapJoin(splitInfo.t1, node), splitInfo.t2);
    ++count;

    return true;
  }

  bool erase(const T &value) {
    auto splitInfo = treapSplitRight(root, value);

    // no element found
    if (!splitInfo.found) {
      root = treapJoin(splitInfo.t1, splitInfo.t2);

      return false;
    }

    // isolate x
    auto eraseElInfo = treapSplit(splitInfo.t2, value);

    assert(eraseElInfo.t1);
    assert(!eraseElInfo.t1->left);
    assert(!eraseElInfo.t1->right);

    // delete node
    delete eraseElInfo.t1;
    --count;

    root = treapJoin(splitInfo.t1, eraseElInfo.t2);
    return true;
  }

  // private:
  static Node *treapJoin(Node *r1, Node *r2) {
    if (!r1) {
      return r2;
    }
    if (!r2) {
      return r1;
    }

    if (r1->weight < r2->weight) {
      r1->right = treapJoin(r1->right, r2);
      r1->right->parent = r1;
      return r1;
    }

    r2->left = treapJoin(r1, r2->left);
    r2->left->parent = r2;
    return r2;
  }

  struct SplitInfo {
    Node *t1;
    Node *t2;
    bool found;
  };

  static SplitInfo treapSplit(Node *r, const T &splitBy) {
    if (!r) {
      return SplitInfo{.t1 = nullptr, .t2 = nullptr, .found = false};
    }

    // r and its right subtree are in T2
    if (splitBy < r->value) {
      auto info = treapSplit(r->left, splitBy);
      r->left = info.t2;

      if (r->left)
        r->left->parent = r;

      r->parent = nullptr;
      return SplitInfo{.t1 = info.t1, .t2 = r, .found = info.found};
    }

    bool found = false;
    // found match
    if (!(r->value < splitBy)) {
      found = true;
    }

    // r and its left subtree is in T1
    auto info = treapSplit(r->right, splitBy);
    r->right = info.t1;

    if (r->right)
      r->right->parent = r;

    r->parent = nullptr;
    return SplitInfo{.t1 = r, .t2 = info.t2, .found = info.found || found};
  }

  static SplitInfo treapSplitRight(Node *r, const T &splitBy) {
    if (!r) {
      return SplitInfo{.t1 = nullptr, .t2 = nullptr, .found = false};
    }

    // r and its left subtree is in T1
    if (r->value < splitBy) {
      auto info = treapSplitRight(r->right, splitBy);
      r->right = info.t1;

      if (r->right)
        r->right->parent = r;

      r->parent = nullptr;
      return SplitInfo{.t1 = r, .t2 = info.t2, .found = info.found};
    }

    bool found = false;
    // found match
    if (!(splitBy < r->value)) {
      found = true;
    }

    // r and its right subtree are in T2
    auto info = treapSplitRight(r->left, splitBy);
    r->left = info.t2;

    if (r->left)
      r->left->parent = r;

    r->parent = nullptr;
    return SplitInfo{.t1 = info.t1, .t2 = r, .found = info.found || found};
  }

public:
  Node *testJoin(std::vector<T> vals) {
    Node *r = new Node(vals[0], nullptr);
    r->weight = 1;

    Node *rl = new Node(vals[1], r);
    rl->weight = 5;

    Node *rr = new Node(vals[2], r);
    rr->weight = 20;

    r->left = rl;
    r->right = rr;

    Node *rrl = new Node(vals[3], rr);
    rrl->weight = 31;
    rr->left = rrl;

    Node *r2 = new Node(vals[4], nullptr);
    r2->weight = 3;

    Node *r2l = new Node(vals[5], r2);
    r2l->weight = 8;

    Node *r2r = new Node(vals[6], r2);
    r2r->weight = 4;

    r2->left = r2l;
    r2->right = r2r;

    Node *res = treapJoin(r, r2);
    return res;
  }

  Node *testInsert(std::vector<T> vals) {
    Node *r = new Node(vals[0], nullptr);
    r->weight = 1;

    insert(r);

    Node *rl = new Node(vals[1], r);
    rl->weight = 5;

    insert(rl);

    Node *rr = new Node(vals[2], r);
    rr->weight = 20;

    insert(rr);

    Node *rrl = new Node(vals[3], rr);
    rrl->weight = 31;

    insert(rrl);

    Node *r2 = new Node(vals[4], nullptr);
    r2->weight = 3;

    insert(r2);

    Node *r2l = new Node(vals[5], r2);
    r2l->weight = 8;

    insert(r2l);

    Node *r2r = new Node(vals[6], r2);
    r2r->weight = 4;

    insert(r2r);

    return root;
  }

  void testErase(std::vector<T> vals) {
    for (int i = 0; i < 7; ++i) {
      erase(vals[i]);
    }
  }
  // Needed to test the structure of the tree.
  // Replace Node with the real type of your nodes
  // and implementations with the ones matching
  // your attributes.
  struct TesterInterface {
    // using Node = ...
    static const Node *root(const Treap *t) { return t->root; }
    // Parent of root must be nullptr, delete if config::PARENT_POINTERS ==
    // false
    static const Node *parent(const Node *n) { return n->parent; }
    static const Node *right(const Node *n) { return n->right; }
    static const Node *left(const Node *n) { return n->left; }
    static const T &value(const Node *n) { return n->value; }
    static uint64_t priority(const Node *n) { return n->weight; }
  };
};

#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
  using std::runtime_error::runtime_error;
};

std::string fmt(const char *f, ...) {
  va_list args1;
  va_list args2;
  va_start(args1, f);
  va_copy(args2, args1);

  std::string buf(vsnprintf(nullptr, 0, f, args1), '\0');
  va_end(args1);

  vsnprintf(buf.data(), buf.size() + 1, f, args2);
  va_end(args2);

  return buf;
}

template <typename T> struct Tester {
  Tester() = default;

  void size() const {
    size_t r = ref.size();
    size_t t = tested.size();
    if (r != t)
      throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
  }

  void find(const T &x) const {
    auto r = ref.find(x);
    auto t = tested.find(x);
    bool found_r = r != nullptr;
    bool found_t = t != nullptr;

    if (found_r != found_t)
      _throw("Find mismatch", found_r);
    if (found_r && *t != x)
      throw TestFailed("Find: found different value");
  }

  void insert(const T &x, bool check_tree_ = false) {
    auto succ_r = ref.insert(x);
    auto succ_t = tested.insert(x);
    if (succ_r != succ_t)
      _throw("Insert mismatch", succ_r);
    size();
    if (check_tree_)
      check_tree();
  }

  void erase(const T &x, bool check_tree_ = false) {
    bool succ_r = ref.erase(x);
    auto succ_t = tested.erase(x);
    if (succ_r != succ_t)
      _throw("Erase mismatch", succ_r);
    size();
    if (check_tree_)
      check_tree();
  }

  struct NodeCheckResult {
    const T *min = nullptr;
    const T *max = nullptr;
    size_t size = 0;
  };

  void check_tree() const {
    using TI = typename Treap<T>::TesterInterface;
    auto ref_it = ref.begin();
    bool check_value_failed = false;
    auto check_value = [&](const T &v) {
      if (check_value_failed)
        return;
      check_value_failed = (ref_it == ref.end() || *ref_it != v);
      if (!check_value_failed)
        ++ref_it;
    };

    auto r = check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr),
                        check_value);
    size_t t_size = tested.size();

    if (t_size != r.size)
      throw TestFailed(fmt("Check tree: size() reports %zu but expected %zu.",
                           t_size, r.size));

    if (check_value_failed)
      throw TestFailed("Check tree: element mismatch");

    size();
  }

  template <typename Node, typename F>
  NodeCheckResult check_node(const Node *n, const Node *p,
                             F &check_value) const {
    if (!n)
      return {};

    using TI = typename Treap<T>::TesterInterface;
    if constexpr (config::PARENT_POINTERS) {
      if (TI::parent(n) != p)
        throw TestFailed("Parent mismatch.");
    }

    if (p && TI::priority(p) > TI::priority(n))
      throw TestFailed("Heap order violated.");

    auto l = check_node(TI::left(n), n, check_value);
    check_value(TI::value(n));
    auto r = check_node(TI::right(n), n, check_value);

    if (l.max && !(*l.max < TI::value(n)))
      throw TestFailed("Max of left subtree is too big.");
    if (r.min && !(TI::value(n) < *r.min))
      throw TestFailed("Min of right subtree is too small.");

    return {l.min ? l.min : &TI::value(n), r.max ? r.max : &TI::value(n),
            1 + l.size + r.size};
  }

  static void _throw(const char *msg, bool s) {
    throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
  }

  Treap<T> tested;
  Ref<T> ref;
};

void test_insert() {
  Tester<int> t;

  for (int i = 0; i < 10; i++)
    t.insert(i, true);
  for (int i = -10; i < 20; i++)
    t.find(i);

  for (int i = 0; i < 10; i++)
    t.insert((1 + i * 7) % 17, true);
  for (int i = -10; i < 20; i++)
    t.find(i);
}

void test_erase() {
  Tester<int> t;

  for (int i = 0; i < 10; i++)
    t.insert((1 + i * 7) % 17, true);
  for (int i = -10; i < 20; i++)
    t.find(i);

  for (int i = 3; i < 22; i += 2)
    t.erase(i, true);
  for (int i = -10; i < 20; i++)
    t.find(i);

  for (int i = 0; i < 10; i++)
    t.insert((1 + i * 13) % 17 - 8, true);
  for (int i = -10; i < 20; i++)
    t.find(i);

  for (int i = -4; i < 10; i++)
    t.erase(i, true);
  for (int i = -10; i < 20; i++)
    t.find(i);
}

enum RandomTestFlags : unsigned { SEQ = 1, NO_ERASE = 2, CHECK_TREE = 4 };

void test_random(size_t size, unsigned flags = 0) {
  Tester<size_t> t;
  std::mt19937 my_rand(24707 + size);

  bool seq = flags & SEQ;
  bool erase = !(flags & NO_ERASE);
  bool check_tree = flags & CHECK_TREE;

  for (size_t i = 0; i < size; i++)
    t.insert(seq ? 2 * i : my_rand() % (3 * size), check_tree);

  t.check_tree();

  for (size_t i = 0; i < 3 * size + 1; i++)
    t.find(i);

  for (size_t i = 0; i < 30 * size; i++)
    switch (my_rand() % 5) {
    case 1:
      t.insert(my_rand() % (3 * size), check_tree);
      break;
    case 2:
      if (erase)
        t.erase(my_rand() % (3 * size), check_tree);
      break;
    default:
      t.find(my_rand() % (3 * size));
    }

  t.check_tree();
}

int main() {
  Treap<int> t;

  Treap<int>::Node *r = t.testJoin({5, 2, 8, 6, 16, 10, 21});
  Treap<int>::Node *parent = nullptr;
  auto ogParent = r;

  assert(r->value == 5 && r->weight == 1 && r->parent == nullptr);

  assert(r->left->value == 2 && r->left->weight == 5 && r->left->parent == r);
  parent = r;
  r = r->right;
  assert(r->value == 16 && r->weight == 3 && r->parent == parent);

  assert(r->right->value == 21 && r->right->weight == 4 &&
         r->right->parent == r);
  parent = r;
  r = r->left;
  assert(r->value == 10 && r->weight == 8 && r->parent == parent);
  parent = r;
  r = r->left;
  assert(r->value == 8 && r->weight == 20 && r->parent == parent);
  parent = r;
  r = r->left;
  assert(r->value == 6 && r->weight == 31 && r->parent == parent);

  auto splitInfo = t.treapSplit(ogParent, 6);

  r = splitInfo.t1;
  assert(r->value == 5 && r->weight == 1 && r->parent == nullptr);
  assert(r->left->value == 2 && r->left->weight == 5 && r->left->parent == r);
  assert(r->right->value == 6 && r->right->weight == 31 &&
         r->right->parent == r);

  r = splitInfo.t2;
  assert(r->value == 16 && r->weight == 3 && r->parent == nullptr);
  assert(r->left->value == 10 && r->left->weight == 8 && r->left->parent == r);
  assert(r->right->value == 21 && r->right->weight == 4 &&
         r->right->parent == r);
  r = r->left;
  assert(r->left->value == 8 && r->left->weight == 20 && r->left->parent == r);

  auto res = t.treapJoin(splitInfo.t1, splitInfo.t2);

  splitInfo = t.treapSplitRight(res, 6);
  r = splitInfo.t1;
  assert(r->value == 5 && r->weight == 1 && r->parent == nullptr);
  assert(r->left->value == 2 && r->left->weight == 5 && r->left->parent == r);
  assert(!r->right);

  r = splitInfo.t2;
  assert(r->value == 16 && r->weight == 3 && r->parent == nullptr);
  assert(r->left->value == 10 && r->left->weight == 8 && r->left->parent == r);
  assert(r->right->value == 21 && r->right->weight == 4 && r->right->parent == r);
  r = r->left;
  assert(r->left->value == 8 && r->left->weight == 20 && r->left->parent == r);
  r = r->left;
  assert(r->left->value == 6 && r->left->weight == 31 && r->left->parent == r);

  res = t.testInsert({5, 2, 8, 6, 16, 10, 21});

  r = t.root;

  assert(r->value == 5 && r->weight == 1 && r->parent == nullptr);

  assert(r->left->value == 2 && r->left->weight == 5 && r->left->parent == r);
  parent = r;
  r = r->right;
  assert(r->value == 16 && r->weight == 3 && r->parent == parent);

  assert(r->right->value == 21 && r->right->weight == 4 &&
         r->right->parent == r);
  parent = r;
  r = r->left;
  assert(r->value == 10 && r->weight == 8 && r->parent == parent);
  parent = r;
  r = r->left;
  assert(r->value == 8 && r->weight == 20 && r->parent == parent);
  parent = r;
  r = r->left;
  assert(r->value == 6 && r->weight == 31 && r->parent == parent);

  t.testErase({5, 2, 8, 6, 16, 10, 21});
  assert(t.size() == 0);
  assert(t.root == nullptr);

  try {
    std::cout << "Insert test..." << std::endl;
    test_insert();

    std::cout << "Erase test..." << std::endl;
    test_erase();

    std::cout << "Tiny random test..." << std::endl;
    test_random(20, CHECK_TREE);

    std::cout << "Small random test..." << std::endl;
    test_random(200, CHECK_TREE);

    std::cout << "Big random test..." << std::endl;
    test_random(10'000);

    std::cout << "Big sequential test..." << std::endl;
    test_random(10'000, SEQ);

    std::cout << "All tests passed." << std::endl;
  } catch (const TestFailed &e) {
    std::cout << "Test failed: " << e.what() << std::endl;
  }

  return 0;
}

#endif
